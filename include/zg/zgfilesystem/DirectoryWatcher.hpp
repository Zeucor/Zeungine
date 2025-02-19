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
using namespace std;
namespace zgfilesystem
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
		DirectoryWatcher(const filesystem::path& path, const vector<filesystem::path>& excludePathsVector);
		vector<pair<ChangeType, string>> update();

	private:
		filesystem::path watchPath;
#ifdef __linux__
		unordered_map<int32_t, filesystem::path> fdPathMap;
#endif
		vector<filesystem::path> excludePathsVector;
		void addDirectoryWatch(const filesystem::path& path);

	public:
		static bool isExcluded(const vector<filesystem::path>& excludePathsVector, const filesystem::path& path);
	};
} // namespace zgfilesystem
