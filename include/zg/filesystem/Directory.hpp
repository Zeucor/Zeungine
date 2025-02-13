#pragma once
#include <string_view>
#include <map>
#include <filesystem>
#include <stdexcept>
namespace zg::filesystem
{
	struct Directory
	{
		const std::filesystem::path path;
		std::map<uint64_t, std::string> entries;
		Directory(std::filesystem::path path);
		bool operator()() const;
		std::map<uint64_t, std::filesystem::path> getRecursiveFileMap() const;
		static bool ensureExists(std::filesystem::path path);
		static std::string getCurrentDirectory();
		static std::string getCurrentDirectoryName();
	};
}