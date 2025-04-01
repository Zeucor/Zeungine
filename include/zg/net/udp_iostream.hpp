#pragma once
#include <istream>
#include <ostream>
#include "udp_streambuf.hpp"

namespace zg::net::streams
{
	class udp_istream : public std::istream
	{
	public:
		explicit udp_istream(const udp_streambuf::SocketPair& fd_addr_pair);
		udp_streambuf buf;
	};

	class udp_ostream : public std::ostream
	{
	public:
		explicit udp_ostream(const udp_streambuf::SocketPair& fd_addr_pair);
		udp_streambuf buf;
	};

	class udp_iostream : public std::iostream
	{
	public:
		explicit udp_iostream(const udp_streambuf::SocketPair& fd_addr_pair);
		udp_streambuf buf;
	};
} // namespace zg
