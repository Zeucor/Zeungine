#include <zg/zgfilesystem/File.hpp>
using namespace zgfilesystem;
using namespace zg::enums;
STANDARD::ios_base::openmode calculateOpenMode(const STANDARD::string& mode, STANDARD::ios_base::openmode openMode = STANDARD::ios::binary)
{
	if (mode.find('r') != STANDARD::string::npos)
	{
		openMode |= STANDARD::ios::in;
	}
	if (mode.find('w') != STANDARD::string::npos)
	{
		openMode |= STANDARD::ios::out | STANDARD::ios::trunc;
	}
	if (mode.find('a') != STANDARD::string::npos)
	{
		openMode |= STANDARD::ios::app;
	}
	if (mode.find('+') != STANDARD::string::npos)
	{
		openMode |= STANDARD::ios::in | STANDARD::ios::out;
	}
	return openMode;
}
File::File(const STANDARD::filesystem::path& _filePath, zg::enums::EFileLocation _fileLocation, const STANDARD::string& mode) :
		AFile(_filePath, _fileLocation, mode), originalFilePath(_filePath), filePath(_filePath),
		fileLocation(_fileLocation), openMode(calculateOpenMode(mode))
{
	if (!open())
	{
		throw STANDARD::runtime_error("Error opening file: " + STANDARD::string(strerror(errno)));
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
		STANDARD::cout << "File '" << filePath << "' failed to close." << STANDARD::endl;
	}
}
bool File::open()
{
	fileStream.open(filePath, openMode);
	return fileStream.is_open();
}
bool File::isOpen()
{
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
	fileStreamRef.seekg(index, STANDARD::ios::beg);
	fileStreamRef.read(static_cast<char*>(pointer), sizeBytes);
	return fileStreamRef.good();
}
bool File::writeBytes(size_t index, size_t sizeBytes, const void* pointer)
{
	if (!fileStream.is_open())
	{
		return false;
	}
	fileStream.seekp(index, STANDARD::ios::beg);
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
	STANDARD::filesystem::resize_file(filePath, newFileSize);
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
		return STANDARD::filesystem::file_size(filePath);
	}
	catch (const STANDARD::filesystem::filesystem_error&)
	{
		return 0;
	}
}
time_t File::lastModified() const
{
	auto ftime = STANDARD::filesystem::last_write_time(filePath);
	auto sctp = STANDARD::chrono::time_point_cast<STANDARD::chrono::system_clock::duration>(ftime - decltype(ftime)::clock::now() +
																																								STANDARD::chrono::system_clock::now());
	return STANDARD::chrono::system_clock::to_time_t(sctp);
}
bool File::remove()
{
	if (STANDARD::filesystem::exists(filePath))
	{
		STANDARD::filesystem::remove(filePath);
		return true;
	}
	return false;
}
STANDARD::filesystem::path File::getUserDirectoryPath() { return STANDARD::filesystem::path(getenv("HOME")); }
STANDARD::filesystem::path File::getProgramDirectoryPath()
{
	STANDARD::filesystem::path exePath;
#if defined(_WIN32)
	char path[MAX_PATH];
	GetModuleFileNameA(NULL, path, MAX_PATH);
	exePath = path;
#elif defined(MACOS)
	char path[1024];
	uint32_t size = sizeof(path);
	if (_NSGetExecutablePath(path, &size) == 0)
		exePath = path;
#elif defined(__linux__)
	exePath = STANDARD::filesystem::canonical("/proc/self/exe");
#endif
	return exePath.parent_path();
}
STANDARD::filesystem::path File::getProgramDataPath() { return STANDARD::filesystem::temp_directory_path(); }
STANDARD::filesystem::path File::getExecutableName() { return STANDARD::filesystem::path(getenv("_")).filename(); }
void replaceSubstring(STANDARD::string& str, const STANDARD::string& from, const STANDARD::string& to)
{
	size_t pos = 0;
	while ((pos = str.find(from, pos)) != STANDARD::string::npos)
	{
		str.replace(pos, from.length(), to);
		pos += to.length();
	}
}
STANDARD::string File::toPlatformPath(STANDARD::string path)
{
	replace(path.begin(), path.end(), '\\', '/');
	STANDARD::filesystem::path fsPath(path);
#ifdef _WIN32
	auto nativePath = STANDARD::wstring_convert<STANDARD::codecvt_utf8<wchar_t>>().to_bytes(fsPath.native().c_str());
#else
	auto nativePath = fsPath.native().c_str();
#endif
	return nativePath;
}
STANDARD::string File::toString()
{
	auto actualSize = size();
	STANDARD::string string;
	string.resize(actualSize);
	readBytes(0, actualSize, string.data());
	return string;
}
STANDARD::shared_ptr<int8_t> File::toBytes()
{
	auto actualSize = size();
	STANDARD::shared_ptr<int8_t> bytes((int8_t*)malloc(actualSize + 1), free);
	memset(bytes.get(), 0, actualSize + 1);
	readBytes(0, actualSize, bytes.get());
	return bytes;
}
