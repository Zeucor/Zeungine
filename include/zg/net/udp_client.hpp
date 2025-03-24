#pragma once
#include "streams/udp_iostream.hpp"
namespace zg::net
{
	struct udp_client : zg::net::streams::udp_iostream
	{
	public:
		udp_client(const std::string& host, int port);

	private:
		streams::udp_streambuf::SocketPair connect(const std::string& host, int port);
	};
} // namespace zg::net
