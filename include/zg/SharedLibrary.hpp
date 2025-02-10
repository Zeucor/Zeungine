#pragma once
#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <dlfcn.h>
#endif
#include <stdexcept>
#include <string>
#include <string_view>
namespace zg
{
	struct SharedLibrary
	{
#ifdef _WIN32
		HMODULE libraryPointer = 0;
#elif defined(__linux__) || defined(__APPLE__)
		void* libraryPointer = 0;
#endif
		template <typename... Args>
		SharedLibrary(const Args&... args)
		{
			std::string paths;
			concatPaths(paths, args...);
			load(paths, args...);
		}
		template <typename... Args>
		void concatPaths(std::string& paths, const std::string& path, const Args&... args)
		{
			paths += std::string(path) + (sizeof...(args) ? ", " : "");
			concatPaths(paths, args...);
		}
		void concatPaths(std::string& paths) {}
		template <typename... Args>
		void load(const std::string& paths, const std::string path, const Args&... args)
		{
#ifdef _WIN32
			libraryPointer = LoadLibraryA(path.data());
#elif defined(__linux__) || defined(__APPLE__)
			libraryPointer = dlopen(path.data(), RTLD_NOW | RTLD_GLOBAL);
#endif
			if (!libraryPointer)
			{
				load(paths, args...);
			}
		}
		void load(const std::string& paths) { throw std::runtime_error("Failed to load librarys: " + paths); }
		~SharedLibrary();
		template <typename T>
		T getProc(std::string_view procName)
		{
			if (!libraryPointer)
			{
				throw std::runtime_error("Library not loaded");
			}
#ifdef _WIN32
			void* procAddress = GetProcAddress(libraryPointer, procName.data());
#elif defined(__linux__) || defined(__APPLE__)
			void* procAddress = dlsym(libraryPointer, procName.data());
#endif
			if (!procAddress)
			{
				throw std::runtime_error("Failed to retrieve proc: " + std::string(procName));
			}
			return reinterpret_cast<T>(procAddress);
		};
		SharedLibrary(const SharedLibrary&) = delete;
		SharedLibrary& operator=(const SharedLibrary&) = delete;
		SharedLibrary(SharedLibrary&& other) noexcept;
		SharedLibrary& operator=(SharedLibrary&& other) noexcept;
	};
} // namespace zg
