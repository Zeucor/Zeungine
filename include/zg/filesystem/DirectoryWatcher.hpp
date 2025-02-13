#pragma once
#include <atomic>
#include <filesystem>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <sys/inotify.h>
#include <unistd.h>
#elif __APPLE__
#include <fcntl.h>
#include <sys/event.h>
#include <sys/types.h>
#include <unistd.h>
#endif
namespace zg::filesystem
{
	class DirectoryWatcher
	{
	public:
		enum ChangeType
		{
			Created,
			Modified,
			Deleted
		};
		DirectoryWatcher(const std::filesystem::path& path, const std::vector<std::filesystem::path>& excludePathsVector);
		std::vector<std::pair<ChangeType, std::string>> update();

	private:
		std::filesystem::path watchPath;
#ifdef __linux__
		std::unordered_map<int32_t, std::filesystem::path> fdPathMap;
#endif
		std::vector<std::filesystem::path> excludePathsVector;
		void addDirectoryWatch(const std::filesystem::path& path);
    public:
		static bool isExcluded(const std::vector<std::filesystem::path>& excludePathsVector,
													 const std::filesystem::path& path);
	};
} // namespace zg::filesystem
