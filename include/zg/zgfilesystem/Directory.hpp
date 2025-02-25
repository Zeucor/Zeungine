#pragma once
#include <zg/Standard.hpp>
namespace zgfilesystem
{
	struct Directory
	{
		const STANDARD::filesystem::path path;
		STANDARD::map<size_t, STANDARD::filesystem::path> entries;
		Directory(STANDARD::filesystem::path path);
		bool operator()() const;
		STANDARD::map<size_t, STANDARD::filesystem::path> getRecursiveFileMap() const;
		static bool ensureExists(STANDARD::filesystem::path path);
		static STANDARD::filesystem::path getCurrentDirectory();
		static STANDARD::filesystem::path getCurrentDirectoryName();
	};
}