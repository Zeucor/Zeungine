#include <zg/filesystem/Directory.hpp>
using namespace zg::filesystem;
Directory::Directory(std::filesystem::path path) : path(path)
{
	if (!std::filesystem::exists(path))
	{
		throw std::runtime_error("Directory does not exist: " + std::string(path));
	}
	for (const auto &entry : std::filesystem::directory_iterator(path))
	{
		entries[std::filesystem::hash_value(entry.path())] = entry.path().string();
	}
}
bool Directory::operator()() const
{
	return std::filesystem::exists(path);
}
std::map<uint64_t, std::filesystem::path> Directory::getRecursiveFileMap() const
{
	std::map<uint64_t, std::filesystem::path> fileMap;
	size_t index = 0;
	for (const auto &entry : std::filesystem::recursive_directory_iterator(path))
	{
		fileMap[++index] = entry.path();
	}
	return fileMap;
}
bool Directory::ensureExists(std::filesystem::path path)
{
	if (!std::filesystem::exists(path))
	{
		return std::filesystem::create_directories(path);
	}
	return true;
}
std::string Directory::getCurrentDirectory()
{
	return std::filesystem::current_path().string();
}
std::string Directory::getCurrentDirectoryName()
{
	return std::filesystem::current_path().filename().string();
}