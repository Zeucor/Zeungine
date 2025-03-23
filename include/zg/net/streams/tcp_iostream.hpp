#pragma once
#include <istream>
#include <ostream>
#include "tcp_streambuf.hpp"

namespace zg::net::streams
{
	class tcp_istream : public std::istream
	{
	public:
		explicit tcp_istream(int socket_fd);

	private:
		tcp_streambuf buf;
	};

	class tcp_ostream : public std::ostream
	{
	public:
		explicit tcp_ostream(int socket_fd);

	private:
		tcp_streambuf buf;
	};

	class tcp_iostream : public std::iostream
	{
	public:
		explicit tcp_iostream(int socket_fd);

	private:
		tcp_streambuf buf;
	};
} // namespace zg
