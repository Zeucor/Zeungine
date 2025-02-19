#pragma once
#include <filesystem>
#include <string>
#include <zg/enums/EFileLocation.hpp>
#define AFile zg::interfaces::IFile
namespace zg::interfaces
{
	struct IFile
	{
	public:
		const std::filesystem::path filePath;
		const enums::EFileLocation fileLocation = enums::EFileLocation::Relative;
		const std::string mode;
		IFile() = default;
		IFile(const std::filesystem::path& filePath, enums::EFileLocation fileLocation = enums::EFileLocation::Relative,
					const std::string& mode = "a+") : filePath(filePath), fileLocation(fileLocation), mode(mode) {};
		virtual ~IFile() = default;
		virtual IFile& operator=(const IFile& other) { return *this; };
		virtual bool open() { return true; };
		virtual bool close() { return true; };
		virtual bool readBytes(size_t index, size_t sizeBytes, void* pointer) { return true; };
		virtual bool writeBytes(size_t index, size_t sizeBytes, const void* pointer) { return true; };
		virtual bool truncate(size_t newIFileSize) { return true; };
		virtual bool sync() { return true; };
		virtual size_t size() const { return 0; };
		virtual time_t lastModified() const { return 0; };
		virtual bool remove() { return true; };
		virtual std::string toString() { return ""; };
		virtual std::shared_ptr<int8_t> toBytes() { return std::shared_ptr<int8_t>(); };
		bool exists(const std::filesystem::path& path) { return std::filesystem::exists(path); }
	};
} // namespace zg::interfaces
