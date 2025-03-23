#pragma once
#include <map>
#include <zg/Events.hpp>
#include <zg/Serial.hpp>
#include "streams/udp_iostream.hpp"
namespace zg::net
{
	struct udp_server
	{
	public:
		using IOStream = std::shared_ptr<zg::net::streams::udp_iostream>;
#if defined(_WIN32)
		using SockLength = int;
#elif defined(__linux__) || defined(MACOS)
		using SockLength = socklen_t;
#endif
	private:
		int port;
		bool bitStream;
		int server_fd = 0;
		std::map<std::pair<uint32_t, uint16_t>, IOStream> clientStreams;

	public:
		udp_server(int port, bool bitStream = false);
		void close();
		IOStream receiveOne();
	};
} // namespace zg::net
