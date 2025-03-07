#pragma once
#include <zg/Standard.hpp>
namespace zg
{
	struct SharedLibrary
	{
		std::string getLastErrorAsString()
		{
#if defined(_WIN32)
			DWORD errorMessageID = GetLastError();
			if (errorMessageID == 0)
				return "No error"; // No error

			LPSTR messageBuffer = nullptr;
			size_t size =
				FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
											 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

			std::string message(messageBuffer, size);
			LocalFree(messageBuffer);
			return message;
#else
			return dlerror();
#endif
		}
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
#elif defined(__linux__) || defined(MACOS)
		void* libraryPointer = 0;
#endif
		template <typename... Args>
		constexpr void load(const STANDARD::string& paths, std::string& errors, const std::filesystem::path& path, const Args&... args)
		{
#ifdef _WIN32
			auto _strn__ = STANDARD::wstring_convert<STANDARD::codecvt_utf8<wchar_t>>().to_bytes(path.native().c_str());
			libraryPointer = LoadLibraryA(_strn__.c_str());
#elif defined(__linux__) || defined(MACOS)
			auto _strn__ = path.string();
			libraryPointer = dlopen(_strn__.c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif
			if (!libraryPointer)
			{
				errors += getLastErrorAsString();
				errors += sizeof...(args) ? ", " : "";
				load(paths, errors, args...);
			}
		}
		void load(const STANDARD::string& paths, STANDARD::string& errors)
		{
			throw STANDARD::runtime_error("Failed to load librarys[" + errors + "]: " + paths);
		}
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
			void* procAddress = (void*)GetProcAddress(libraryPointer, procName.data());
#elif defined(__linux__) || defined(MACOS)
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
