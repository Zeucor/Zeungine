#include <zg/strings/HookedConsole.hpp>
#ifdef MACOS
#include <unistd.h>
#endif
using namespace zg::strings;
HookedConsole::HookedConsole(const std::function<void(const std::vector<std::string> &)> &_outputCallback, StdHandleToRedirect handle) : outputCallback(_outputCallback),
                                                                                                                                         m_handle(handle)
{
    initializeRedirect();
}
HookedConsole::~HookedConsole()
{
    stop();
}
void HookedConsole::stop()
{
    isRunning = false;
    if (readerThread.joinable())
    {
        readerThread.join();
    }
}
std::vector<std::string> HookedConsole::getAllLines()
{
    std::lock_guard<std::mutex> lock(outputMutex);
    return allLines;
}
void HookedConsole::initializeRedirect()
{
#ifdef _WIN32
    CreatePipe(&readablePipeEnd, &writablePipeEnd, nullptr, 0);
    SetStdHandle(m_handle == STDOUT ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE, writablePipeEnd);
    int writablePipeEndFileStream = _open_osfhandle(reinterpret_cast<intptr_t>(writablePipeEnd), 0);
    FILE *writablePipeEndFile = _fdopen(writablePipeEndFileStream, "wt");
    _dup2(_fileno(writablePipeEndFile), m_handle == STDOUT ? STDOUT_FILENO : STDERR_FILENO);
#elif defined(LINUX) || defined(MACOS)
    int32_t pipeEnds[2];
    if (pipe(pipeEnds) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    readablePipeEnd = pipeEnds[0];
    writablePipeEnd = pipeEnds[1];
    if (m_handle == STDOUT)
    {
        if (dup2(writablePipeEnd, STDOUT_FILENO) == -1)
        {
            throw std::runtime_error("dup2 error");
        }
    }
    else
    {
        if (dup2(writablePipeEnd, STDERR_FILENO) == -1)
        {
            throw std::runtime_error("dup2 error");
        }
    }
    close(writablePipeEnd);
#endif
    readerThread = std::thread(&HookedConsole::readFromPipe, this);
}
void HookedConsole::readFromPipe()
{
    const size_t bufferSize = 1024;
    char buffer[bufferSize];
    unsigned long bytesRead;
    while (isRunning)
    {
#ifdef _WIN32
        DWORD bytesAvailable = 0;
        if (!PeekNamedPipe(readablePipeEnd, nullptr, 0, nullptr, &bytesAvailable, nullptr) || bytesAvailable == 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        if (ReadFile(readablePipeEnd, buffer, bufferSize, &bytesRead, nullptr) && bytesRead > 0)
        {
            processBuffer(buffer, bytesRead);
        }
#elif defined(LINUX) || defined(MACOS)
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(readablePipeEnd, &readSet);
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;
        int32_t result = select(readablePipeEnd + 1, &readSet, nullptr, nullptr, &timeout);
        if (result > 0 && FD_ISSET(readablePipeEnd, &readSet))
        {
            ssize_t bytes = read(readablePipeEnd, buffer, bufferSize);
            if (bytes > 0)
            {
                processBuffer(buffer, static_cast<unsigned long>(bytes));
            }
            else if (bytes == 0)
            {
                break;
            }
            else if (errno != EAGAIN && errno != EINTR)
            {
                throw std::runtime_error("read error");
                break;
            }
        }
        else if (result < 0)
        {
            throw std::runtime_error("select error");
            break;
        }
#endif
    }
}
void HookedConsole::processBuffer(const char *buffer, size_t length)
{
    std::lock_guard<std::mutex> lock(outputMutex);
    std::vector<std::string> thisLines;
    for (size_t i = 0; i < length; ++i)
    {
        char c = buffer[i];
        if (c == '\n')
        {
            thisLines.push_back(currentLine);
            allLines.push_back(currentLine);
            currentLine.clear();
        }
        else if (c != '\r')
        {
            currentLine += c;
        }
    }
    outputCallback(thisLines);
}