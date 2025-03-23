#pragma once
#include <istream>
#include <ostream>
#include "udp_streambuf.hpp"

namespace zg::net::streams
{
	class udp_istream : public std::istream
	{
	public:
		explicit udp_istream(int server_fd, sockaddr_in addr);

	private:
		udp_streambuf buf;
	};

	class udp_ostream : public std::ostream
	{
	public:
		explicit udp_ostream(int server_fd, sockaddr_in addr);

		void pushData(const char* data, size_t length);

	private:
		udp_streambuf buf;
	};

	class udp_iostream : public std::iostream
	{
	public:
		explicit udp_iostream(int server_fd, sockaddr_in addr);

		void pushData(const char* data, size_t length);

	private:
		udp_streambuf buf;
	};
} // namespace zg
