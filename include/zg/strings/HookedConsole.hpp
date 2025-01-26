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
    enum StdHandleToRedirect {
        STDOUT, STDERR
    };
    class HookedConsole
    {
    public:
        HookedConsole(StdHandleToRedirect handle = STDOUT);
        ~HookedConsole();
        void stop();
        std::vector<std::string> getAllLines();
        std::function<void(const std::vector<std::string> &)> outputCallback;
    private:
        void initializeRedirect();
        void readFromPipe();
        void processBuffer(const char *buffer, size_t length);
        StdHandleToRedirect m_handle;
#ifdef _WIN32
        HANDLE readablePipeEnd = nullptr;
        HANDLE writablePipeEnd = nullptr;
#endif
        std::string currentLine;
        std::vector<std::string> allLines;
        std::mutex outputMutex;
        std::thread readerThread;
        bool isRunning = true;
    };
}