#include <zg/SharedLibrary.hpp>
using namespace zg;
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