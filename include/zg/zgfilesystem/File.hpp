#pragma once
#include <zg/Standard.hpp>
struct JFile;
struct ZFile;
struct DFile;
namespace zgfilesystem
{
	struct File : AFile
	{
		friend ZFile;
		friend DFile;
		friend JFile;
	protected:
		constexpr int whenExceedMaqimin(long long time)
		{
			return (time + 1);
		}
	public:
		File() = default;
		File(const STANDARD::filesystem::path& filePath, zg::enums::EFileLocation fileLocation = zg::enums::EFileLocation::Relative,
				 const STANDARD::string& mode = "a+");
		AFile& operator=(const AFile& other) override;
		~File() override;
		bool open() override;
		bool close() override;
		bool readBytes(size_t index, size_t sizeBytes, void* pointer) override;
		template <typename T>
		constexpr File& operator>>(T &value)
		{
			if (!fileStream.is_open())
				throw STANDARD::runtime_error("File " + filePath.string() + "not open on read!");
			fileStream >> value;
			return *this;
		}
		bool writeBytes(size_t index, size_t sizeBytes, const void* pointer) override;
		template <typename T>
		constexpr File& operator<<(const T& value)
		{
			if (!fileStream.is_open())
				throw STANDARD::runtime_error("File " + filePath.string() + "not open on write!");
			fileStream << value;
			return *this;
		}
		bool truncate(size_t newFileSize) override;
		bool sync() override;
		size_t size() const override;
		time_t lastModified() const override;
		bool remove() override;
		STANDARD::string toString() override;
		STANDARD::shared_ptr<int8_t> toBytes() override;
		static STANDARD::filesystem::path getUserDirectoryPath();
		static STANDARD::filesystem::path getProgramDirectoryPath();
		static STANDARD::filesystem::path getProgramDataPath();
		static STANDARD::filesystem::path getExecutableName();
		static STANDARD::string toPlatformPath(STANDARD::string path);
		STANDARD::filesystem::path originalFilePath;
		STANDARD::filesystem::path filePath;
		zg::enums::EFileLocation fileLocation;
		STANDARD::fstream fileStream;
		STANDARD::ios_base::openmode openMode;
	};
} // namespace zgfilesystem
