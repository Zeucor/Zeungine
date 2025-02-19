#include <chrono>
#include <codecvt>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string.h>
#include <zg/zgfilesystem/File.hpp>
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#endif
using namespace zgfilesystem;
using namespace zg::enums;
ios_base::openmode calculateOpenMode(const string& mode, ios_base::openmode openMode = ios::binary)
{
	if (mode.find('r') != string::npos)
	{
		openMode |= ios::in;
	}
	if (mode.find('w') != string::npos)
	{
		openMode |= ios::out | ios::trunc;
	}
	if (mode.find('a') != string::npos)
	{
		openMode |= ios::app;
	}
	if (mode.find('+') != string::npos)
	{
		openMode |= ios::in | ios::out;
	}
	return openMode;
}
File::File(const filesystem::path& _filePath, zg::enums::EFileLocation _fileLocation, const string& mode) :
		AFile(_filePath, _fileLocation, mode), originalFilePath(_filePath), filePath(_filePath),
		fileLocation(_fileLocation), openMode(calculateOpenMode(mode))
{
	if (!open())
	{
		throw runtime_error("Error opening file: " + string(strerror(errno)));
	}
}
AFile& File::operator=(const AFile& other)
{
	if (this != &other)
	{
		filePath = other.filePath;
		fileLocation = other.fileLocation;
		openMode = calculateOpenMode(other.mode);
		if (fileStream.is_open())
		{
			fileStream.close();
		}
	}
	return *this;
}
File::~File()
{
	sync();
	if (!close())
	{
		cout << "File '" << filePath << "' failed to close." << endl;
	}
}
bool File::open()
{
	fileStream.open(filePath, openMode);
	return fileStream.is_open();
}
bool File::close()
{
	if (fileStream.is_open())
	{
		fileStream.close();
		return !fileStream.is_open();
	}
	return false;
}
bool File::readBytes(size_t index, size_t sizeBytes, void* pointer)
{
	auto& fileStreamRef = fileStream;
	if (!fileStreamRef.is_open())
	{
		return false;
	}
	fileStreamRef.seekg(index, ios::beg);
	fileStreamRef.read(static_cast<char*>(pointer), sizeBytes);
	return fileStreamRef.good();
}
bool File::writeBytes(size_t index, size_t sizeBytes, const void* pointer)
{
	if (!fileStream.is_open())
	{
		return false;
	}
	fileStream.seekp(index, ios::beg);
	fileStream.write(static_cast<const char*>(pointer), sizeBytes);
	return fileStream.good();
}
bool File::truncate(size_t newFileSize)
{
	if (!fileStream.is_open())
	{
		return false;
	}
	fileStream.close();
	filesystem::resize_file(filePath, newFileSize);
	return open();
}
bool File::sync()
{
	if (fileStream.is_open())
	{
		fileStream.flush();
		return true;
	}
	return false;
}
size_t File::size() const
{
	try
	{
		return filesystem::file_size(filePath);
	}
	catch (const filesystem::filesystem_error&)
	{
		return 0;
	}
}
time_t File::lastModified() const
{
	auto ftime = filesystem::last_write_time(filePath);
	auto sctp = chrono::time_point_cast<chrono::system_clock::duration>(ftime - decltype(ftime)::clock::now() +
																																								chrono::system_clock::now());
	return chrono::system_clock::to_time_t(sctp);
}
bool File::remove()
{
	if (filesystem::exists(filePath))
	{
		filesystem::remove(filePath);
		return true;
	}
	return false;
}
filesystem::path File::getUserDirectoryPath() { return filesystem::path(getenv("HOME")); }
filesystem::path File::getProgramDirectoryPath()
{
	filesystem::path exePath;
#if defined(_WIN32)
	char path[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);
	exePath = path;
#elif defined(__APPLE__)
	char path[1024];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0)
		exePath = path;
#elif defined(__linux__)
	exePath = filesystem::canonical("/proc/self/exe");
#endif
	return exePath.parent_path();
}
filesystem::path File::getProgramDataPath() { return filesystem::temp_directory_path(); }
string File::getExecutableName() { return filesystem::path(getenv("_")).filename(); }
void replaceSubstring(string& str, const string& from, const string& to)
{
	size_t pos = 0;
	while ((pos = str.find(from, pos)) != string::npos)
	{
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
}
string File::toPlatformPath(string path)
{
	replace(path.begin(), path.end(), '\\', '/');
	filesystem::path fsPath(path);
#ifdef _WIN32
	auto nativePath = wstring_convert<codecvt_utf8<wchar_t>>().to_bytes(fsPath.native().c_str());
#else
	auto nativePath = fsPath.native().c_str();
#endif
	return nativePath;
}
string File::toString()
{
	auto actualSize = size();
	string string;
	string.resize(actualSize);
	readBytes(0, actualSize, string.data());
	return string;
}
shared_ptr<int8_t> File::toBytes()
{
	auto actualSize = size();
	shared_ptr<int8_t> bytes((int8_t*)malloc(actualSize + 1), free);
	memset(bytes.get(), 0, actualSize + 1);
	readBytes(0, actualSize, bytes.get());
	return bytes;
}
