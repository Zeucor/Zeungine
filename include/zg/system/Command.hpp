#include <iostream>
#include <string>
#include <cstdlib>
#include <array>
#include <memory>

#ifdef _WIN32
    #include <windows.h>
    #include <io.h>
    #include <fcntl.h>
    using ProcessHandle = HANDLE;
#else
    #include <cstdio>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    #include <signal.h>
    using ProcessHandle = pid_t;
#endif

namespace zg::system
{
    class Command
    {
    private:
        ProcessHandle handle;
        int exitCode = 0;
        bool complete = false;
        int pipeRead = 0;
    public:
        Command(const std::string &command);
        bool update();    
        void kill();    
        bool isComplete() const;
        int getExitCode() const;
    };
}
