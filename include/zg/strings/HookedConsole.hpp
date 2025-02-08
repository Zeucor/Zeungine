#pragma once
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif
#include <functional>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <string>

namespace zg::strings
{
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
    enum StdHandleToRedirect
    {
        STDOUT,
        STDERR
    };
    class HookedConsole
    {
    public:
        HookedConsole(const std::function<void(const std::vector<std::string> &)> &outputCallback, StdHandleToRedirect handle = STDOUT);
        ~HookedConsole();
        void stop();
        std::vector<std::string> getAllLines();

    private:
        std::function<void(const std::vector<std::string> &)> outputCallback;
        void initializeRedirect();
        void readFromPipe();
        void processBuffer(const char *buffer, size_t length);
        StdHandleToRedirect m_handle;
#ifdef _WIN32
        HANDLE readablePipeEnd = nullptr;
        HANDLE writablePipeEnd = nullptr;
#elif defined(LINUX) || defined(MACOS)
        int32_t readablePipeEnd = 0;
        int32_t writablePipeEnd = 0;
#endif
        std::string currentLine;
        std::vector<std::string> allLines;
        std::mutex outputMutex;
        std::thread readerThread;
        bool isRunning = true;
    };
}