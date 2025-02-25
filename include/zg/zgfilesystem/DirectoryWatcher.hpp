#pragma once
#include <zg/Standard.hpp>
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
		DirectoryWatcher(const STANDARD::filesystem::path& path, const STANDARD::vector<STANDARD::filesystem::path>& excludePathsVector);
		STANDARD::vector<STANDARD::pair<ChangeType, STANDARD::filesystem::path>> update();

	private:
		STANDARD::filesystem::path watchPath;
#ifdef __linux__
		STANDARD::unordered_map<int32_t, STANDARD::filesystem::path> fdPathMap;
#endif
		STANDARD::vector<STANDARD::filesystem::path> excludePathsVector;
		void addDirectoryWatch(const STANDARD::filesystem::path& path);

	public:
		static bool isExcluded(const STANDARD::vector<STANDARD::filesystem::path>& excludePathsVector, const STANDARD::filesystem::path& path);
	};
} // namespace zgfilesystem
