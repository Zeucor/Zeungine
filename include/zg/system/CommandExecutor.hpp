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
	struct CommandProcessor
	{
		static void execute(const std::string& command)
		{
			std::array<char, 128> buffer;
			FILE* pipe = POPEN(command.c_str(), "r");
			if (!pipe)
			{
				std::cerr << "Failed to execute command." << std::endl;
				return;
			}
			while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
			{
				std::cout << buffer.data();
			}
			PCLOSE(pipe);
		}
	};
} // namespace zg::system
