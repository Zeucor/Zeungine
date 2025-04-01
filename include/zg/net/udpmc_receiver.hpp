#pragma once
#include "udp_iostream.hpp"
namespace zg::net
{
	struct udpmc_receiver : zg::net::streams::udp_istream
	{
	public:
		udpmc_receiver(const std::string& host, int port);

	private:
		streams::udp_streambuf::SocketPair bind(const std::string& host, int port);
	};
} // namespace zg::net
