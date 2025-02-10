#pragma once
#include <array>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#ifdef _WIN32
#include <windows.h>
#define POPEN _popen
#define PCLOSE _pclose
#else
#include <cstdio>
#include <unistd.h>
#define POPEN popen
#define PCLOSE pclose
#endif
namespace zg::system
{
	struct CommandExecutor
	{
		static int32_t execute(const std::string& command);
	};
} // namespace zg::system
