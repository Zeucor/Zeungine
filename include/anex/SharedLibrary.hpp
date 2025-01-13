#pragma once
#ifdef _WIN32
#include <windows.h>
#endif
#include <string_view>
#include <stdexcept>
namespace anex
{
	struct SharedLibrary
	{
	#ifdef _WIN32
		HMODULE libraryHandle;
	#endif
		explicit SharedLibrary(std::string_view path);
		~SharedLibrary();
		template<typename T>
		T getProc(std::string_view procName)
		{
			if (!libraryHandle)
			{
				throw std::runtime_error("Library not loaded");
			}
	#ifdef _WIN32
			void* procAddress = GetProcAddress(libraryHandle, procName.data());
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
}