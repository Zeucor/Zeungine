#pragma once
#include <string_view>
#include <map>
#include <filesystem>
#include <stdexcept>
using namespace std;
namespace zgfilesystem
{
	struct Directory
	{
		const std::filesystem::path path;
		std::map<size_t, std::filesystem::path> entries;
		Directory(std::filesystem::path path);
		bool operator()() const;
		std::map<size_t, std::filesystem::path> getRecursiveFileMap() const;
		static bool ensureExists(std::filesystem::path path);
		static std::filesystem::path getCurrentDirectory();
		static std::filesystem::path getCurrentDirectoryName();
	};
}