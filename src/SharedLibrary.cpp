#include <anex/SharedLibrary.hpp>
using namespace anex;
SharedLibrary::SharedLibrary(std::string_view path)
		: libraryHandle(nullptr)
{
#ifdef _WIN32
	libraryHandle = LoadLibraryA(path.data());
#endif
	if (!libraryHandle)
	{
		throw std::runtime_error("Failed to load library: " + std::string(path));
	}
};
SharedLibrary::~SharedLibrary()
{
	if (libraryHandle)
	{
#ifdef _WIN32
		FreeLibrary(libraryHandle);
#endif
	}
};
SharedLibrary::SharedLibrary(SharedLibrary&& other) noexcept
		: libraryHandle(other.libraryHandle)
{
	other.libraryHandle = nullptr;
};
SharedLibrary& SharedLibrary::operator=(SharedLibrary&& other) noexcept
{
	if (this != &other)
	{
		if (libraryHandle)
		{
#ifdef _WIN32
			FreeLibrary(libraryHandle);
#endif
		}

		libraryHandle = other.libraryHandle;
		other.libraryHandle = nullptr;
	}
	return *this;
};