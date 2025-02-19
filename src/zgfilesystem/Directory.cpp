#include <zg/zgfilesystem/Directory.hpp>
using namespace zgfilesystem;
Directory::Directory(filesystem::path path) : path(path)
{
	if (!filesystem::exists(path))
	{
		throw runtime_error("Directory does not exist: " + string(path));
	}
	size_t index = 0;
	for (const auto &entry : filesystem::directory_iterator(path))
	{
		entries[++index] = entry.path();
	}
}
bool Directory::operator()() const
{
	return filesystem::exists(path);
}
map<size_t, filesystem::path> Directory::getRecursiveFileMap() const
{
	map<size_t, filesystem::path> fileMap;
	size_t index = 0;
	for (const auto &entry : filesystem::recursive_directory_iterator(path))
	{
		fileMap[++index] = entry.path();
	}
	return fileMap;
}
bool Directory::ensureExists(filesystem::path path)
{
	if (!filesystem::exists(path))
	{
		return filesystem::create_directories(path);
	}
	return true;
}
filesystem::path Directory::getCurrentDirectory()
{
	return filesystem::current_path();
}
filesystem::path Directory::getCurrentDirectoryName()
{
	return filesystem::current_path().filename();
}