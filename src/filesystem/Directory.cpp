#include <zg/filesystem/Directory.hpp>
using namespace zg::filesystem;
Directory::Directory(std::string_view path):
	path(path)
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
std::map<uint64_t, std::string> Directory::getRecursiveFileMap() const
{
	std::map<uint64_t, std::string> fileMap;
	for (const auto &entry : std::filesystem::recursive_directory_iterator(path))
	{
		fileMap[std::filesystem::hash_value(entry.path())] = entry.path().string();
	}
	return fileMap;
}
bool Directory::ensureExists(std::string_view path)
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