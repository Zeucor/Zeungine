#include <fcntl.h>
#include <string.h>
#include <zg/zgfilesystem/Directory.hpp>
#include <zg/zgfilesystem/DirectoryWatcher.hpp>
using namespace zgfilesystem;
DirectoryWatcher::DirectoryWatcher(const std::filesystem::path& path,
																	 const std::vector<std::filesystem::path>& excludePathsVector) :
		watchPath(path), excludePathsVector(excludePathsVector)
{
	if (!std::filesystem::exists(watchPath) || !std::filesystem::is_directory(watchPath))
	{
		throw std::runtime_error("Invalid directory path.");
	}
	Directory directory(watchPath.c_str());
	auto recursiveFileMap = directory.getRecursiveFileMap();
	addDirectoryWatch(path);
	for (auto& filePair : recursiveFileMap)
		if (std::filesystem::is_directory(filePair.second))
			addDirectoryWatch(filePair.second);
}
std::vector<std::pair<DirectoryWatcher::ChangeType, std::string>> DirectoryWatcher::update()
{
	std::vector<std::pair<ChangeType, std::string>> changes;
#ifdef _WIN32
	HANDLE dirHandle =
		CreateFileW(watchPath.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
								OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, nullptr);

	if (dirHandle == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error("Failed to open directory for watching.");
	}

	char buffer[1024];
	DWORD bytesReturned;
	FILE_NOTIFY_INFORMATION* eventInfo;

	while (ReadDirectoryChangesW(dirHandle, buffer, sizeof(buffer), TRUE,
															 FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
															 &bytesReturned, nullptr, nullptr))
	{
		eventInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
		std::wstring fileName(eventInfo->FileName, eventInfo->FileNameLength / sizeof(WCHAR));
		callback(watchPath / std::filesystem::path(fileName), Modified);
	}

	CloseHandle(dirHandle);

#elif __linux__
	char buffer[1024];
	for (auto& fdPathPair : fdPathMap)
	{
		auto& fd = fdPathPair.first;
		while (true)
		{
			ssize_t length = read(fd, buffer, sizeof(buffer));
			if (length == -1)
			{
				if (errno == EAGAIN)
				{
					break;
				}
				else
				{
					throw std::runtime_error("Failed to read inotify events.");
				}
			}
			for (ssize_t i = 0; i < length;)
			{
				struct inotify_event* event = reinterpret_cast<struct inotify_event*>(&buffer[i]);
				if (event->len)
				{
					ChangeType changeType;
					if (event->mask & IN_CREATE)
						changeType = ChangeType::Created;
					if (event->mask & IN_MODIFY)
						changeType = ChangeType::Modified;
					if (event->mask & IN_DELETE)
						changeType = ChangeType::Deleted;
					changes.push_back({changeType, fdPathPair.second / event->name});
					if (event->mask & IN_CREATE && std::filesystem::is_directory(watchPath / event->name))
					{
						addDirectoryWatch(fdPathPair.second / event->name);
					}
				}
				i += sizeof(struct inotify_event) + event->len;
			}
		}
	}
#elif __APPLE__
	int kq = kqueue();
	if (kq == -1)
		throw std::runtime_error("Failed to create kqueue.");

	int dirFD = open(watchPath.c_str(), O_RDONLY);
	if (dirFD == -1)
	{
		close(kq);
		throw std::runtime_error("Failed to open directory.");
	}

	struct kevent change;
	EV_SET(&change, dirFD, EVFILT_VNODE, EV_ADD | EV_ENABLE | EV_CLEAR,
				 NOTE_WRITE | NOTE_EXTEND | NOTE_ATTRIB | NOTE_DELETE | NOTE_RENAME, 0, nullptr);

	struct kevent event;
	while (kevent(kq, &change, 1, &event, 1, nullptr) > 0)
	{
		changed(ChangeType::Modified, watchPath);
	}

	close(dirFD);
	close(kq);
#endif
	return changes;
}
void DirectoryWatcher::addDirectoryWatch(const std::filesystem::path& path)
{
	if (std::filesystem::is_directory(path))
	{
		if (isExcluded(excludePathsVector, path))
			return;
#ifdef __linux__
		auto fd = inotify_init();
		if (fd < 0)
			throw std::runtime_error("Failed to initialize inotify.");

		int wd = inotify_add_watch(fd, path.c_str(), IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
		if (wd < 0)
		{
			close(fd);
			throw std::runtime_error("Failed to add inotify watch.");
		}
		int flags = fcntl(fd, F_GETFL, 0);
		if (flags == -1)
		{
			close(fd);
			throw std::runtime_error("Failed to get file descriptor flags.");
		}
		if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		{
			close(fd);
			throw std::runtime_error("Failed to set file descriptor to non-blocking.");
		}
		fdPathMap[fd] = path;
	}
#endif
}
bool DirectoryWatcher::isExcluded(const std::vector<std::filesystem::path>& excludePathsVector, const std::filesystem::path& path)
{
	for (auto& excludePath : excludePathsVector)
	{
		if (strncmp(path.c_str(), excludePath.c_str(), strlen(excludePath.c_str())) == 0)
			return true;
	}
	return false;
}
