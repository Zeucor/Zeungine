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
	class udp_streambuf : public std::streambuf
	{
	public:
#if defined(_WIN32)
		using SocketIdentifier = SOCKET;
#elif defined(__linux__) || defined(MACOS)
		using SocketIdentifier = int;
#endif
		explicit udp_streambuf(SocketIdentifier server_fd, sockaddr_in client_addr);

		~udp_streambuf();

		void setReceivedData(const char* data, size_t length);

	protected:
		int underflow() override;

		int overflow(int c = traits_type::eof()) override;

		int sync() override;

	private:
		SocketIdentifier server_fd;
		sockaddr_in client_addr;
		std::vector<char> buffer;
	};
} // namespace zg::net::streams
