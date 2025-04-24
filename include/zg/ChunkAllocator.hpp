#pragma once
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <new>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
namespace zg
{
	// Default size for chunks allocated by the global manager
	constexpr size_t GLOBAL_MANAGER_CHUNK_SIZE = 1024 * 1024;
	struct GlobalChunk
	{
		char* memory = nullptr;
		size_t size = 0;

		GlobalChunk(size_t requested_size) : size(requested_size)
		{
			if (size == 0)
				throw std::invalid_argument("GlobalChunk size cannot be zero.");
			memory = static_cast<char*>(::operator new(size, std::align_val_t{alignof(std::max_align_t)}));
			memset(memory, 0, size);
		}

		~GlobalChunk()
		{
			if (memory)
			{
				::operator delete(memory, size, std::align_val_t{alignof(std::max_align_t)});
			}
		}

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

	class GlobalAllocatorManager
	{
	private:
		static bool initialize(size_t initial_chunk_size = GLOBAL_MANAGER_CHUNK_SIZE)
		{
			std::lock_guard<std::mutex> lock(s_mutex);
			if (s_initialized || s_shutdown)
			{
				if (s_initialized)
					throw std::runtime_error("GlobalAllocatorManager already initialized.");
				if (s_shutdown)
				{
					s_global_chunks.clear();
					s_current_pos = nullptr;
					s_remaining_size = 0;
					s_shutdown = false;
				}
			}
			try
			{
				allocate_new_global_chunk(initial_chunk_size);
				s_initialized = true;
				return true;
			}
			catch (...)
			{
				s_global_chunks.clear();
				s_current_pos = nullptr;
				s_remaining_size = 0;
				s_initialized = false;
				s_shutdown = false;
				throw;
			}
		}
		static void shutdown()
		{
			std::lock_guard<std::mutex> lock(s_mutex);
			if (!s_initialized && !s_shutdown)
				return;

			s_global_chunks.clear();
			s_current_pos = nullptr;
			s_remaining_size = 0;
			s_initialized = false;
			s_shutdown = true;
		}

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
		GlobalAllocatorManager() = delete;
		~GlobalAllocatorManager() = delete;

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
			size_t total_needed = 0;

			while (true)
			{
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
						throw std::bad_alloc();
					}
					total_needed = requested_size + padding;

					if (total_needed <= s_remaining_size)
					{
						char* result_ptr = aligned_ptr;
						s_current_pos = aligned_ptr + requested_size;
						s_remaining_size -= total_needed;
						return {result_ptr, requested_size};
					}
				}

				size_t min_needed_for_chunk = 0;
				size_t align_max = alignof(std::max_align_t);
				if (requested_size > (std::numeric_limits<size_type>::max)() - (align_max - 1))
				{
					throw std::bad_alloc();
				}
				min_needed_for_chunk = requested_size + align_max - 1;
				allocate_new_global_chunk(min_needed_for_chunk);
			}
		}
	};

	template <typename T>
	class ChunkAllocator
	{
	public:
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
			using other = ChunkAllocator<U>;
		};

		// --- Propagation Traits ---
		// Still independent instances, traits remain false.
		using propagate_on_container_copy_assignment = std::false_type;
		using propagate_on_container_move_assignment = std::false_type;
		using propagate_on_container_swap = std::false_type;
		using is_always_equal = std::false_type;

		ChunkAllocator() noexcept = default;
		ChunkAllocator(const ChunkAllocator&) noexcept = default;
		ChunkAllocator(ChunkAllocator&&) noexcept = default;

		template <typename U>
		ChunkAllocator(const ChunkAllocator<U>& /*other*/) noexcept
		{
		}

		ChunkAllocator& operator=(const ChunkAllocator&) noexcept = default;
		ChunkAllocator& operator=(ChunkAllocator&&) noexcept = default;

		~ChunkAllocator() = default;

		[[nodiscard]] pointer allocate(size_type n)
		{
			if (n == 0)
				return nullptr;
			if (n > (std::numeric_limits<size_type>::max)() / sizeof(T))
			{
				throw std::bad_alloc();
			}
			const size_type bytes_needed = n * sizeof(T);
			std::pair<char*, size_t> result_block = GlobalAllocatorManager::request_block(bytes_needed);
			assert(result_block.first != nullptr);
			assert(result_block.second >= bytes_needed);
			return reinterpret_cast<pointer>(result_block.first);
		}

		void deallocate(pointer p, size_type n) noexcept
		{
			(void)p;
			(void)n;
		}

		template <typename U>
		bool operator==(const ChunkAllocator<U>& other) const noexcept
		{
			return this == &other;
		}

		template <typename U>
		bool operator!=(const ChunkAllocator<U>& other) const noexcept
		{
			return !(*this == other);
		}

	}; // class ChunkAllocator
} // namespace zg
