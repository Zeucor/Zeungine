#pragma once
#include <string_view>
#include <map>
#include <filesystem>
#include <stdexcept>
namespace zg::filesystem
{
	struct Directory
	{
		const std::string_view path;
		std::map<uint64_t, std::string> entries;
		Directory(std::string_view path);
		bool operator()() const;
		std::map<uint64_t, std::string> getRecursiveFileMap() const;
		static bool ensureExists(std::string_view path);
		static std::string getCurrentDirectory();
		static std::string getCurrentDirectoryName();
	};
}