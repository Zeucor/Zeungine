#pragma once
#include <string>
#ifdef _WIN32
#include <windows.h>
#define SET_WORKING_DIR(path)                                                                                          \
	if (!SetCurrentDirectory(path))                                                                                      \
	throw std::runtime_error("Failed to set working directory: " + std::string(path))
#else
#include <cstdio>
#include <unistd.h>
#define SET_WORKING_DIR(path)                                                                                          \
	if (chdir(path) != 0)                                                                                                \
	throw std::runtime_error("Failed to set working directory: " + std::string(path))
#endif
