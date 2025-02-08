#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <ctime>
#include <optional>
#include <zg/enums/EFileLocation.hpp>

namespace zg::filesystem
{
	struct File
	{
	public:
		File() = default;
		File(const std::string &filePath, enums::EFileLocation fileLocation = enums::EFileLocation::Relative, const std::string &mode = "a+");
		File &operator=(const File &other);
		~File();
		bool open();
		bool close();
		bool readBytes(size_t index, size_t sizeBytes, void *pointer);
		bool writeBytes(size_t index, size_t sizeBytes, const void *pointer);
		bool truncate(size_t newFileSize);
		bool sync();
		size_t size();
		bool exists() const;
		time_t lastModified() const;
		bool remove();
		std::string toString();
		std::shared_ptr<int8_t[]> toBytes();
		static bool exists(const std::string &path);
		static std::string getUserDirectoryPath();
		static std::string getProgramDirectoryPath();
		static std::string getProgramDataPath();
		static std::string getExecutableName();
		static std::string toPlatformPath(std::string path);
		std::string originalFilePath;
		std::string filePath;
		enums::EFileLocation fileLocation;
		std::fstream fileStream;
		std::ios_base::openmode openMode;
	};
}