#pragma once
#include <ctime>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <zg/enums/EFileLocation.hpp>
#include <zg/interfaces/IFile.hpp>
namespace zg::filesystem
{
	struct File : AFile
	{
	public:
		File() = default;
		File(const std::filesystem::path& filePath, enums::EFileLocation fileLocation = enums::EFileLocation::Relative,
				 const std::string& mode = "a+");
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
		std::string toString() override;
		std::shared_ptr<int8_t> toBytes() override;
		static std::filesystem::path getUserDirectoryPath();
		static std::filesystem::path getProgramDirectoryPath();
		static std::filesystem::path getProgramDataPath();
		static std::string getExecutableName();
		static std::string toPlatformPath(std::string path);
		std::filesystem::path originalFilePath;
		std::filesystem::path filePath;
		enums::EFileLocation fileLocation;
		std::fstream fileStream;
		std::ios_base::openmode openMode;
	};
} // namespace zg::filesystem
