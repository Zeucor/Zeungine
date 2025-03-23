#pragma once
#include "streams/tcp_iostream.hpp"
namespace zg::net
{
	struct tcp_client : zg::net::streams::tcp_iostream
	{
	private:
        std::string host;
        int port;
	public:
		tcp_client(const std::string& host, int port);

	private:
		int connect(const std::string& host, int port);
	};
} // namespace zg::net
