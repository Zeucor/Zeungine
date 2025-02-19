#pragma once
#include <ctime>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <zg/enums/EFileLocation.hpp>
#include <zg/interfaces/IFile.hpp>
using namespace std;
namespace zgfilesystem
{
	struct File : AFile
	{
	public:
		File() = default;
		File(const filesystem::path& filePath, zg::enums::EFileLocation fileLocation = zg::enums::EFileLocation::Relative,
				 const string& mode = "a+");
		AFile& operator=(const AFile& other) override;
		~File() override;
		bool open() override;
		bool close() override;
		bool readBytes(size_t index, size_t sizeBytes, void* pointer) override;
		bool writeBytes(size_t index, size_t sizeBytes, const void* pointer) override;
		bool truncate(size_t newFileSize) override;
		bool sync() override;
		size_t size() const override;
		time_t lastModified() const override;
		bool remove() override;
		string toString() override;
		shared_ptr<int8_t> toBytes() override;
		static filesystem::path getUserDirectoryPath();
		static filesystem::path getProgramDirectoryPath();
		static filesystem::path getProgramDataPath();
		static string getExecutableName();
		static string toPlatformPath(string path);
		filesystem::path originalFilePath;
		filesystem::path filePath;
		zg::enums::EFileLocation fileLocation;
		fstream fileStream;
		ios_base::openmode openMode;
	};
} // namespace zgfilesystem
