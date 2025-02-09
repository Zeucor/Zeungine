#pragma once
#ifdef _WIN32
#include <windows.h>
#elif defined(LINUX) || defined(MACOS)
#include <dlfcn.h>
#endif
#include <string_view>
#include <stdexcept>
#include <string>
namespace zg
{
	struct SharedLibrary
	{
#ifdef _WIN32
		HMODULE libraryPointer = 0;
#elif defined(LINUX) || defined(MACOS)
		void *libraryPointer = 0;
#endif
		template <typename... Args>
		SharedLibrary(Args... args)
		{
			load(args...);
		}
		template <typename... Args>
		void load(std::string_view path, Args... args)
		{
#ifdef _WIN32
			libraryPointer = LoadLibraryA(path.data());
#elif defined(LINUX) || defined(MACOS)
			libraryPointer = dlopen(path.data(), RTLD_NOW | RTLD_GLOBAL);
#endif
			if (!libraryPointer)
			{
				load(args...);
			}
		}
		void load()
		{
			throw std::runtime_error("Failed to load library");
		}
		~SharedLibrary();
		template <typename T>
		T getProc(std::string_view procName)
		{
			if (!libraryPointer)
			{
				throw std::runtime_error("Library not loaded");
			}
#ifdef _WIN32
			void *procAddress = GetProcAddress(libraryPointer, procName.data());
#elif defined(LINUX) || defined(MACOS)
			void *procAddress = dlsym(libraryPointer, procName.data());
#endif
			if (!procAddress)
			{
				throw std::runtime_error("Failed to retrieve proc: " + std::string(procName));
			}
			return reinterpret_cast<T>(procAddress);
		};
		SharedLibrary(const SharedLibrary &) = delete;
		SharedLibrary &operator=(const SharedLibrary &) = delete;
		SharedLibrary(SharedLibrary &&other) noexcept;
		SharedLibrary &operator=(SharedLibrary &&other) noexcept;
	};
}