#include <anex/strings/HookedConsole.hpp>
using namespace anex::strings;
HookedConsole::HookedConsole(StdHandleToRedirect handle):
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
#endif
        {
            processBuffer(buffer, bytesRead);
        }
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