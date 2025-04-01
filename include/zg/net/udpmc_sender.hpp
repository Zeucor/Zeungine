#pragma once
#include <map>
#include <zg/Events.hpp>
#include <zg/Serial.hpp>
#include "udp_iostream.hpp"
namespace zg::net
{
	struct udpmc_sender : streams::udp_ostream
	{
	public:
#if defined(_WIN32)
		using SockLength = int;
#elif defined(__linux__) || defined(MACOS)
		using SockLength = socklen_t;
#endif
	private:
		int port;
		bool bitStream;
		int server_fd = 0;

	public:
		udpmc_sender(const std::string& host, int port);

	private:
		streams::udp_streambuf::SocketPair setup(const std::string& host, int port);
		void close();
	};
} // namespace zg::net
