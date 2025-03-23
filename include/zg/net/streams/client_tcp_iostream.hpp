#pragma once
#include <istream>
#include <ostream>
#include "tcp_streambuf.hpp"

namespace zg::net::streams
{
	class client_tcp_istream : public std::istream
	{
	public:
		explicit client_tcp_istream(int socket_fd);

	private:
		tcp_streambuf buf;
	};

	class client_tcp_ostream : public std::ostream
	{
	public:
		explicit client_tcp_ostream(int socket_fd);

	private:
		tcp_streambuf buf;
	};

	class client_tcp_iostream : public std::iostream
	{
	public:
		explicit client_tcp_iostream(int socket_fd);

	private:
		tcp_streambuf buf;
	};
} // namespace zg
