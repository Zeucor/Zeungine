#include <zg/SharedLibrary.hpp>
using namespace zg;
SharedLibrary::SharedLibrary(std::string_view path)
	: libraryPointer(nullptr)
{
#ifdef _WIN32
	libraryPointer = LoadLibraryA(path.data());
#elif defined(LINUX) || defined(MACOS)
	libraryPointer = dlopen(path.data(), RTLD_NOW | RTLD_GLOBAL);
#endif
	if (!libraryPointer)
	{
		throw std::runtime_error("Failed to load library: " + std::string(path));
	}
}
SharedLibrary::~SharedLibrary()
{
	if (libraryPointer)
	{
#ifdef _WIN32
		FreeLibrary(libraryPointer);
#elif defined(LINUX) || defined(MACOS)
		dlclose(libraryPointer);
#endif
	}
}
SharedLibrary::SharedLibrary(SharedLibrary &&other) noexcept
	: libraryPointer(other.libraryPointer)
{
	other.libraryPointer = nullptr;
}
SharedLibrary &SharedLibrary::operator=(SharedLibrary &&other) noexcept
{
	if (this != &other)
	{
		if (libraryPointer)
		{
#ifdef _WIN32
			FreeLibrary(libraryPointer);
#elif defined(LINUX) || defined(MACOS)
			dlclose(libraryPointer);
#endif
		}

		libraryPointer = other.libraryPointer;
		other.libraryPointer = nullptr;
	}
	return *this;
}