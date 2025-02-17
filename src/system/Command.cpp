#include <zg/system/Command.hpp>
using namespace zg::system;
Command::Command(const std::string& command) : pid(0), exitCode(0), complete(false), pipeRead(-1)
{
	std::cout << "[Executing command]: " << command << std::endl;
#ifdef _WIN32
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	HANDLE hRead, hWrite;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
		std::cerr << "Failed to create pipe." << std::endl;
		pid = NULL;
		return;
	}

	STARTUPINFO si = {sizeof(STARTUPINFO)};
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = hWrite;
	si.hStdError = hWrite;
	si.hStdInput = NULL;

	PROCESS_INFORMATION pi;
	if (!CreateProcess(NULL, const_cast<char*>(command.c_str()), NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si,
										 &pi))
	{
		std::cerr << "Failed to execute command." << std::endl;
		CloseHandle(hRead);
		CloseHandle(hWrite);
		pid = NULL;
		return;
	}
	pid = pi.hProcess;
	CloseHandle(pi.hThread);
	CloseHandle(hWrite);
	pipeRead = _open_osfpid(reinterpret_cast<intptr_t>(hRead), _O_RDONLY);
#else
	int pipefd[2];
	if (pipe(pipefd) == -1)
	{
		std::cerr << "Failed to create pipe." << std::endl;
		pid = -1;
		return;
	}
	pid = fork();
	if (pid == 0)
	{
		close(pipefd[0]);
		dup2(pipefd[1], STDOUT_FILENO);
		dup2(pipefd[1], STDERR_FILENO);
		close(pipefd[1]);
		execl("/bin/sh", "sh", "-c", command.c_str(), (char*)NULL);
		exit(EXIT_FAILURE);
	}
	else if (pid < 0)
	{
		std::cerr << "Failed to fork process." << std::endl;
		pid = -1;
		return;
	}
	close(pipefd[1]);
	pipeRead = pipefd[0];
#endif
}

bool Command::update()
{
	if (pipeRead != -1)
	{
		char buffer[128];
#ifdef _WIN32
		DWORD bytesRead;
		if (PeekNamedPipe(reinterpret_cast<HANDLE>(_get_osfpid(pipeRead)), NULL, 0, NULL, &bytesRead, NULL) &&
				bytesRead > 0)
		{
			if (ReadFile(reinterpret_cast<HANDLE>(_get_osfpid(pipeRead)), buffer, sizeof(buffer) - 1, &bytesRead, NULL) &&
					bytesRead > 0)
			{
				buffer[bytesRead] = '\0';
				std::cout << buffer;
			}
		}
#else
		ssize_t bytesRead = read(pipeRead, buffer, sizeof(buffer) - 1);
		if (bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			std::cout << buffer;
		}
#endif
	}

#ifdef _WIN32
	if (pid)
	{
		DWORD status;
		if (GetExitCodeProcess(pid, &status) && status != STILL_ACTIVE)
		{
			exitCode = static_cast<int>(status);
			CloseHandle(pid);
			complete = true;
		}
	}
#else
	if (pid > 0)
	{
		int status;
		pid_t result = waitpid(pid, &status, WNOHANG);
		if (result == pid)
		{
			if (WIFEXITED(status))
			{
				exitCode = WEXITSTATUS(status);
			}
			complete = true;
		}
	}
#endif
	return complete;
}

void Command::kill()
{
#ifdef _WIN32
	if (pid)
	{
		TerminateProcess(pid, 0);
		CloseHandle(pid);
		complete = true;
	}
#else
	if (pid > 0)
	{
		::kill(pid, SIGKILL);
		complete = true;
	}
#endif
}

bool Command::isComplete() const { return complete; }
int Command::getExitCode() const { return exitCode; }
