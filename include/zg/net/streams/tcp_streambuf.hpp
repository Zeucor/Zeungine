#pragma once
#include <streambuf>
#include <vector>
#if defined(__linux__) || defined(MACOS)
#include <sys/socket.h>
#include <unistd.h>
#elif defined(_WIN32)
#include <windows.h>
#endif
namespace zg::net::streams
{
	class tcp_streambuf : public std::streambuf
	{
	public:
		explicit tcp_streambuf(int socket_fd, std::size_t buffer_size = 4096);

		~tcp_streambuf();

	protected:
		int underflow() override;

		int overflow(int c = traits_type::eof()) override;

		int sync() override;

	private:
#if defined(_WIN32)
		using SocketIdentifier = SOCKET;
#elif defined(__linux__) || defined(MACOS)
		using SocketIdentifier = int;
#endif
		SocketIdentifier socket_fd;
		std::vector<char> buffer;
	};
} // namespace zg
