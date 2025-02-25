#pragma once
#include <string>
#include <stdexcept>
#ifdef _WIN32
#include <windows.h>
void SET_WORKING_DIR(LPCSTR path)
{
	if (!SetCurrentDirectoryA(path))
	    throw std::runtime_error("Failed to set working directory: " + std::string(path));
}
#else
#include <cstdio>
#include <unistd.h>
void SET_WORKING_DIR(const char *path)
{
	if (chdir(path) != 0)
    	throw std::runtime_error("Failed to set working directory: " + std::string(path))
}
#endif
#include <string>
#ifdef _WIN32
#include <vector>
#include <memory>
inline std::string GET_WORKING_DIR() 
{
    DWORD length = GetCurrentDirectoryA(0, nullptr);  // Get required buffer length
    if (length == 0) 
    {
        throw std::runtime_error("Failed to get current working directory");
    }
    std::vector<char> buffer(length + 1, 0);
    if (GetCurrentDirectoryA(length, buffer.data()) == 0) 
    {
        throw std::runtime_error("Failed to get current working directory");
    }
    return std::string(buffer.data());
}
#else
#include <limits.h>
inline std::string GET_WORKING_DIR() 
{
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) == nullptr) 
    {
        throw std::runtime_error("Failed to get current working directory");
    }
    return std::string(buffer);
}
#endif