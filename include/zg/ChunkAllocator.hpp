#pragma once

#include <algorithm> // std::max
#include <atomic> // std::atomic (for potential future thread-safe init flag)
#include <cassert> // assert
#include <cstddef> // size_t, std::ptrdiff_t, std::max_align_t
#include <limits> // std::numeric_limits
#include <list> // std::list
#include <memory> // std::pointer_traits
#include <mutex> // std::mutex, std::lock_guard
#include <new> // ::operator new, std::bad_alloc, std::align_val_t
#include <stdexcept> // std::runtime_error, std::invalid_argument
#include <string> // std::string (for exceptions)
#include <type_traits> // std::true_type, std::false_type, std::is_same
#include <utility> // std::move
#include <vector> // std::vector (for usage examples)


// Note on Thread Safety:
// - GlobalAllocatorManager uses a mutex for thread-safe block requests.
// - ChunkAllocator<T> instances themselves are NOT inherently thread-safe if the
//   same instance is used across threads. Each thread should ideally use its
//   own ChunkAllocator<T> instance, which will then safely request blocks
//   from the global manager when needed.

namespace zg
{
	// Default size for chunks allocated by the global manager
	constexpr size_t GLOBAL_MANAGER_CHUNK_SIZE = 1024 * 1024; // 1 MiB
	// REMOVED: ALLOCATOR_DEFAULT_BLOCK_REQUEST_SIZE


	// --- Global Raw Chunk Structure (Managed by GlobalAllocatorManager) ---
	// [ Unchanged from previous version - Keeping for context ]
	struct GlobalChunk
	{
		char* memory = nullptr;
		size_t size = 0;

		GlobalChunk(size_t requested_size) : size(requested_size)
		{
			if (size == 0)
				throw std::invalid_argument("GlobalChunk size cannot be zero.");
			memory = static_cast<char*>(::operator new(size, std::align_val_t{alignof(std::max_align_t)}));
		}

		~GlobalChunk()
		{
			if (memory)
			{
				::operator delete(memory, size, std::align_val_t{alignof(std::max_align_t)});
			}
		}

		// Non-copyable, Movable
		GlobalChunk(const GlobalChunk&) = delete;
		GlobalChunk& operator=(const GlobalChunk&) = delete;
		GlobalChunk(GlobalChunk&& other) noexcept : memory(other.memory), size(other.size)
		{
			other.memory = nullptr;
			other.size = 0;
		}
		GlobalChunk& operator=(GlobalChunk&& other) noexcept
		{
			if (this != &other)
			{
				if (memory)
					::operator delete(memory, size, std::align_val_t{alignof(std::max_align_t)});
				memory = other.memory;
				size = other.size;
				other.memory = nullptr;
				other.size = 0;
			}
			return *this;
		}
	};

	// --- Global Allocator Manager (Static Class) ---
	// [ Unchanged from previous version - Keeping for context ]
	class GlobalAllocatorManager
	{
	private:
		// Initialize the global pool (call once at startup)
		static bool initialize(size_t initial_chunk_size = GLOBAL_MANAGER_CHUNK_SIZE)
		{
			std::lock_guard<std::mutex> lock(s_mutex);
			if (s_initialized || s_shutdown)
			{
				if (s_initialized)
					throw std::runtime_error("GlobalAllocatorManager already initialized.");
				// Reset state if previously shut down before re-initializing
				if (s_shutdown)
				{
					s_global_chunks.clear();
					s_current_pos = nullptr;
					s_remaining_size = 0;
					s_shutdown = false; // Allow re-initialization
				}
			}
			try
			{
				allocate_new_global_chunk(initial_chunk_size);
				s_initialized = true; // Set initialized flag here
				return true;
			}
			catch (...)
			{
				// Ensure cleanup if initial allocation fails
				s_global_chunks.clear();
				s_current_pos = nullptr;
				s_remaining_size = 0;
				s_initialized = false; // Ensure not marked as initialized
				s_shutdown = false;
				throw; // Re-throw exception
			}
		}
		// Shutdown the global pool (call once at teardown)
		static void shutdown()
		{
			std::lock_guard<std::mutex> lock(s_mutex);
			if (!s_initialized && !s_shutdown) // Only return if never initialized and not already shutdown
				return;

			s_global_chunks.clear(); // Destructors of GlobalChunk free memory
			s_current_pos = nullptr;
			s_remaining_size = 0;
			s_initialized = false;
			s_shutdown = true;
		}
		// Static members holding the global state
		inline static std::list<GlobalChunk> s_global_chunks = {};
		inline static char* s_current_pos = 0;
		inline static size_t s_remaining_size = 0;
		inline static std::mutex s_mutex = std::mutex{};
		inline static bool s_initialized = ([](){
            std::atexit(&shutdown);
            return initialize(GLOBAL_MANAGER_CHUNK_SIZE);
        })();
		inline static bool s_shutdown = false;
		using size_type = size_t;

		// Private helper to align pointers
		static inline char* align_global_ptr(char* ptr) noexcept
		{
			const size_t alignment = alignof(std::max_align_t);
			std::uintptr_t int_ptr = reinterpret_cast<std::uintptr_t>(ptr);
			std::uintptr_t aligned_int_ptr = (int_ptr + alignment - 1) / alignment * alignment;
			return reinterpret_cast<char*>(aligned_int_ptr);
		}

		// Private helper to add a new chunk to the global pool (must be called within lock)
		static void allocate_new_global_chunk(size_t min_size_needed)
		{
			size_t size_to_allocate = (std::max)(GLOBAL_MANAGER_CHUNK_SIZE, min_size_needed);
			s_global_chunks.emplace_front(size_to_allocate); // Throws bad_alloc on failure

			GlobalChunk& new_chunk = s_global_chunks.front();
			s_current_pos = new_chunk.memory; // Start from the beginning
			s_remaining_size = new_chunk.size; // All space is available
		}

	public:
		// Deleted constructor/destructor - static class
		GlobalAllocatorManager() = delete;
		~GlobalAllocatorManager() = delete;

		// Request a block of raw memory from the global pool (thread-safe)
		// Returns the start pointer and the actual size allocated.
		static std::pair<char*, size_t> request_block(size_t requested_size)
		{
			if (requested_size == 0)
				return {nullptr, 0};

			std::lock_guard<std::mutex> lock(s_mutex);
			if (!s_initialized)
				s_initialized = initialize();
			if (s_shutdown)
				throw std::runtime_error("GlobalAllocatorManager has been shut down.");


			char* aligned_ptr = nullptr;
			size_t padding = 0;
			size_t total_needed = 0; // requested_size + padding

			while (true)
			{ // Loop until allocation succeeds or throws
				if (s_current_pos)
				{
					aligned_ptr = align_global_ptr(s_current_pos);
					// Check for overflow/wrap-around with padding calculation
					if (aligned_ptr < s_current_pos)
					{
						throw std::runtime_error("GlobalAllocatorManager internal error: Pointer alignment wrap-around.");
					}
					padding = static_cast<size_t>(aligned_ptr - s_current_pos);

					// Check for overflow when adding padding
					if (requested_size > (std::numeric_limits<size_type>::max)() - padding)
					{
						throw std::bad_alloc(); // Request too large with padding
					}
					total_needed = requested_size + padding;

					if (total_needed <= s_remaining_size)
					{
						// Found space in the current global chunk
						char* result_ptr = aligned_ptr;
						s_current_pos = aligned_ptr + requested_size; // Advance past allocated block
						s_remaining_size -= total_needed; // Decrease remaining
						return {result_ptr, requested_size}; // Return aligned ptr and the size *requested*
					}
				}

				// Need a new global chunk (or first allocation)
				// Allocate a chunk large enough for this request + alignment padding
				// Ensure minimum size calculation doesn't overflow
				size_t min_needed_for_chunk = 0;
				size_t align_max = alignof(std::max_align_t);
				if (requested_size > (std::numeric_limits<size_type>::max)() - (align_max - 1))
				{
					throw std::bad_alloc(); // Cannot calculate minimum chunk size needed
				}
				min_needed_for_chunk = requested_size + align_max - 1;
				allocate_new_global_chunk(min_needed_for_chunk);
				// Loop will retry allocation from the new chunk
			}
		}
	};

	// --- ChunkAllocator (Uses Global Manager Directly for Each Allocation) ---
	// *** MODIFIED: Removed BlockRequestSize template parameter ***
	template <typename T>
	class ChunkAllocator
	{
	private:
		// *** MODIFIED: Removed all local block state members ***
		// char* m_current_block_start = nullptr;
		// char* m_current_pos = nullptr;
		// size_t m_remaining_in_block = 0;

		// *** MODIFIED: Removed acquire_new_block helper ***

	public:
		// --- Standard Allocator Typedefs ---
		using value_type = T;
		using pointer = T*;
		using const_pointer = const T*;
		using reference = T&;
		using const_reference = const T&;
		using size_type = size_t;
		using difference_type = std::ptrdiff_t;

		template <typename U>
		struct rebind
		{
			// *** MODIFIED: Removed BlockRequestSize ***
			using other = ChunkAllocator<U>;
		};

		// --- Propagation Traits ---
		// Still independent instances, traits remain false.
		using propagate_on_container_copy_assignment = std::false_type;
		using propagate_on_container_move_assignment = std::false_type;
		using propagate_on_container_swap = std::false_type;
		using is_always_equal = std::false_type;

		// --- Constructors ---
		// Allocator is now effectively stateless regarding memory blocks.
		ChunkAllocator() noexcept = default;
		ChunkAllocator(const ChunkAllocator&) noexcept = default; // Copy is trivial
		ChunkAllocator(ChunkAllocator&&) noexcept = default; // Move is trivial

		// Templated constructor (for rebinding): Creates a new, independent instance.
		template <typename U>
		ChunkAllocator(const ChunkAllocator<U>& /*other*/) noexcept
		{
			// No state to copy or share.
		}

		// --- Assignment Operators ---
		// Also trivial now.
		ChunkAllocator& operator=(const ChunkAllocator&) noexcept = default;
		ChunkAllocator& operator=(ChunkAllocator&&) noexcept = default;

		// Destructor: Default is fine. Memory belongs to Global Manager.
		~ChunkAllocator() = default;


		// --- Core Allocation/Deallocation ---
		[[nodiscard]] pointer allocate(size_type n)
		{
			if (n == 0)
				return nullptr;
			if (n > (std::numeric_limits<size_type>::max)() / sizeof(T))
			{
				throw std::bad_alloc(); // Request size overflow
			}
			const size_type bytes_needed = n * sizeof(T);
			// const size_type alignment = alignof(T); // Alignment handled globally

			// *** MODIFIED: Directly request exact size from Global Manager ***
			// The Global Manager returns memory aligned to at least alignof(std::max_align_t).
			// This is sufficient for T if alignof(T) <= alignof(std::max_align_t).
			// If T is over-aligned, this allocator will not provide correct alignment.
			std::pair<char*, size_t> result_block = GlobalAllocatorManager::request_block(bytes_needed);

			// request_block throws on failure or if not initialized/shutdown.
			// It returns {nullptr, 0} only if requested_size was 0, handled above.
			assert(result_block.first != nullptr);
			assert(result_block.second >= bytes_needed); // Manager returns at least requested size

			return reinterpret_cast<pointer>(result_block.first);
		}

		// Deallocate remains a no-op (memory managed globally)
		void deallocate(pointer p, size_type n) noexcept
		{
			(void)p;
			(void)n;
		}

		// --- Comparison Operators ---
		template <typename U> // *** MODIFIED: Removed BlockRequestSize ***
		bool operator==(const ChunkAllocator<U>& other) const noexcept
		{
			// Stateless allocators can be considered equal, but standard practice
			// for non-always-equal allocators is identity for non-shared state.
			// Let's keep identity comparison for consistency with is_always_equal = false.
			return this == &other;
		}

		template <typename U> // *** MODIFIED: Removed BlockRequestSize ***
		bool operator!=(const ChunkAllocator<U>& other) const noexcept
		{
			return !(*this == other);
		}

	}; // class ChunkAllocator

	// --- Equality for different types (needed by standard) ---
	// If both are stateless ChunkAllocators, maybe they should compare true?
	// But sticking to is_always_equal = false means identity comparison is safer.
	template <typename T1, typename T2>
	bool operator==(const ChunkAllocator<T1>& lhs, const ChunkAllocator<T2>& rhs) noexcept
	{
		return static_cast<const void*>(&lhs) == static_cast<const void*>(&rhs);
	}

	template <typename T1, typename T2>
	bool operator!=(const ChunkAllocator<T1>& lhs, const ChunkAllocator<T2>& rhs) noexcept
	{
		return !(lhs == rhs);
	}


} // namespace zg
