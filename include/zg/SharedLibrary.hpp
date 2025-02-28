#pragma once
#include <zg/Standard.hpp>
namespace zg
{
	struct SharedLibrary
	{
		template <typename T, typename... Args>
		constexpr void concatPaths(STANDARD::string& paths, const T& path, const Args&... args)
		{
			if constexpr (STANDARD::is_same_v<T, STANDARD::string>)
				paths += path;
			else if constexpr (std::is_same_v<T, std::filesystem::path>)
				paths += path.string();
			else
				paths += STANDARD::to_string(path);
			paths += (sizeof...(args) ? ", " : "");
			concatPaths(paths, args...);
		}
		void concatPaths(STANDARD::string& paths) {}
#ifdef _WIN32
		HMODULE libraryPointer = 0;
#elif defined(__linux__) || defined(__APPLE__)
		void* libraryPointer = 0;
#endif
	template <typename T, typename... Args>
	constexpr void load(const STANDARD::string& paths, std::string &errors, const T& path, const Args&... args)
	{
		if constexpr (STANDARD::is_same_v<T, STANDARD::string> || STANDARD::is_same_v<T, STANDARD::string_view>)
		{
			#ifdef _WIN32
			libraryPointer = LoadLibraryA(path.data());
			#elif defined(__linux__) || defined(__APPLE__)
			libraryPointer = dlopen(path.data(), RTLD_NOW | RTLD_GLOBAL);
			#endif
		}
		else
		{
			throw std::runtime_error("unknown load T path");
		}
		if (!libraryPointer)
		{
			errors += dlerror();
			load(paths, errors, args...);
		}
	}
	void load(const STANDARD::string& paths, STANDARD::string &errors) { throw STANDARD::runtime_error("Failed to load librarys[" + errors + "]: " + paths); }
		template <typename... Args>
		SharedLibrary(const Args&... args)
		{
			STANDARD::string paths;
			STANDARD::string errors;
			concatPaths(paths, args...);
			load(paths, errors, args...);
		}
		~SharedLibrary();
		template <typename T>
		T getProc(STANDARD::string_view procName)
		{
			if (!libraryPointer)
			{
				throw STANDARD::runtime_error("Library not loaded");
			}
#ifdef _WIN32
			void* procAddress = GetProcAddress(libraryPointer, procName.data());
#elif defined(__linux__) || defined(__APPLE__)
			void* procAddress = dlsym(libraryPointer, procName.data());
#endif
			if (!procAddress)
			{
				throw STANDARD::runtime_error("Failed to retrieve proc: " + STANDARD::string(procName));
			}
			return reinterpret_cast<T>(procAddress);
		};
		SharedLibrary(const SharedLibrary&) = delete;
		SharedLibrary& operator=(const SharedLibrary&) = delete;
		SharedLibrary(SharedLibrary&& other) noexcept;
		SharedLibrary& operator=(SharedLibrary&& other) noexcept;
	};
} // namespace zg
