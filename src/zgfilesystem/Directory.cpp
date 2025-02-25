#include <zg/zgfilesystem/Directory.hpp>
using namespace zgfilesystem;
Directory::Directory(STANDARD::filesystem::path path) : path(path)
{
	if (!STANDARD::filesystem::exists(path))
	{
		throw STANDARD::runtime_error("Directory does not exist: " + path.string());
	}
	size_t index = 0;
	for (const auto &entry : STANDARD::filesystem::directory_iterator(path))
	{
		entries[++index] = entry.path();
	}
}
bool Directory::operator()() const
{
	return STANDARD::filesystem::exists(path);
}
STANDARD::map<size_t, STANDARD::filesystem::path> Directory::getRecursiveFileMap() const
{
	STANDARD::map<size_t, STANDARD::filesystem::path> fileMap;
	size_t index = 0;
	for (const auto &entry : STANDARD::filesystem::recursive_directory_iterator(path))
	{
		fileMap[++index] = entry.path();
	}
	return fileMap;
}
bool Directory::ensureExists(STANDARD::filesystem::path path)
{
	if (!STANDARD::filesystem::exists(path))
	{
		return STANDARD::filesystem::create_directories(path);
	}
	return true;
}
STANDARD::filesystem::path Directory::getCurrentDirectory()
{
	return STANDARD::filesystem::current_path();
}
STANDARD::filesystem::path Directory::getCurrentDirectoryName()
{
	return STANDARD::filesystem::current_path().filename();
}