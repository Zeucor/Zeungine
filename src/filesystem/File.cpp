#include <zg/filesystem/File.hpp>
#include <chrono>
#include <codecvt>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string.h>
using namespace zg::filesystem;
File::File(const std::string &_filePath, enums::EFileLocation _fileLocation, const std::string &mode)
    : originalFilePath(_filePath), filePath(_filePath), fileLocation(_fileLocation), openMode(std::ios::in | std::ios::out | std::ios::binary)
{
    if (mode.find('r') != std::string::npos)
    {
        openMode |= std::ios::in;
    }
    if (mode.find('w') != std::string::npos)
    {
        openMode |= std::ios::out | std::ios::trunc;
    }
    if (mode.find('a') != std::string::npos)
    {
        openMode |= std::ios::app;
    }
    if (mode.find('+') != std::string::npos)
    {
        openMode |= std::ios::in | std::ios::out;
    }
    if (!open())
    {
        throw std::runtime_error("Error opening file: " + std::string(strerror(errno)));
    }
};
File &File::operator=(const File &other)
{
    if (this != &other)
    {
        originalFilePath = other.originalFilePath;
        filePath = other.filePath;
        fileLocation = other.fileLocation;
        openMode = other.openMode;
        if (fileStream.is_open())
        {
            fileStream.close();
        }
    }
    return *this;
};
File::~File()
{
    sync();
    if (!close())
    {
        std::cout << "File '" << filePath << "' failed to close." << std::endl;
    }
};
bool File::open()
{
    fileStream.open(filePath, openMode);
    return fileStream.is_open();
};
bool File::close()
{
    if (fileStream.is_open())
    {
        fileStream.close();
        return !fileStream.is_open();
    }
    return false;
};
bool File::readBytes(size_t index, size_t sizeBytes, void *pointer)
{
    auto& fileStreamRef = fileStream;
    if (!fileStreamRef.is_open())
    {
        return false;
    }
    fileStreamRef.seekg(index, std::ios::beg);
    fileStreamRef.read(static_cast<char *>(pointer), sizeBytes);
    return fileStreamRef.good();
};
bool File::writeBytes(size_t index, size_t sizeBytes, const void *pointer)
{
    if (!fileStream.is_open())
    {
        return false;
    }
    fileStream.seekp(index, std::ios::beg);
    fileStream.write(static_cast<const char *>(pointer), sizeBytes);
    return fileStream.good();
};
bool File::truncate(size_t newFileSize)
{
    if (!fileStream.is_open())
    {
        return false;
    }
    fileStream.close();
    std::filesystem::resize_file(filePath, newFileSize);
    return open();
};
bool File::sync()
{
    if (fileStream.is_open())
    {
        fileStream.flush();
        return true;
    }
    return false;
};
size_t File::size() const
{
    try
    {
        return std::filesystem::file_size(filePath);
    }
    catch (const std::filesystem::filesystem_error &)
    {
        return 0;
    }
};
bool File::exists() const
{
    return std::filesystem::exists(filePath);
};
time_t File::lastModified() const
{
    auto ftime = std::filesystem::last_write_time(filePath);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - decltype(ftime)::clock::now() + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(sctp);
};
bool File::remove()
{
    if (std::filesystem::exists(filePath))
    {
        std::filesystem::remove(filePath);
        return true;
    }
    return false;
};
bool File::exists(const std::string &path)
{
    return std::filesystem::exists(path);
};
std::string File::getUserDirectoryPath()
{
    return std::filesystem::path(getenv("HOME")).string();
};
std::string File::getProgramDirectoryPath()
{
    return std::filesystem::current_path().string();
};
std::string File::getProgramDataPath()
{
    return std::filesystem::temp_directory_path().string();
};
std::string File::getExecutableName()
{
    return std::filesystem::path(getenv("_"))
        .filename()
        .string();
};
void replaceSubstring(std::string &str, const std::string &from, const std::string &to)
{
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos)
    {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
}
std::string File::toPlatformPath(std::string path)
{
    std::replace(path.begin(), path.end(), '\\', '/');
    std::filesystem::path fsPath(path);
#ifdef _WIN32
    auto nativePath = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(fsPath.native().c_str());
#else
    auto nativePath = fsPath.native().c_str();
#endif
    return nativePath;
};
std::string File::toString()
{
    auto actualSize = size();
    std::string string;
    string.resize(actualSize);
    readBytes(0, actualSize, string.data());
    return string;
};
std::shared_ptr<int8_t[]> File::toBytes()
{
    auto actualSize = size();
    std::shared_ptr<int8_t[]> bytes = std::shared_ptr<int8_t[]>(new int8_t[actualSize]);
    readBytes(0, actualSize, bytes.get());
    return bytes;
};