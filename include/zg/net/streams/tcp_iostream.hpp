#pragma once
#include <istream>
#include <ostream>
#include "tcp_streambuf.hpp"
#include "openssl/ssl.h"
#include "openssl/err.h"
using SSLPair = std::pair<SSL_CTX*, SSL*>;
namespace zg::net::streams
{
	class tcp_istream : public std::istream
	{
	public:
		explicit tcp_istream(const std::pair<int, SSL*>& fd_ssl_pair);

		void SSLUpgrade(SSL* sslPointer);

	private:
		tcp_streambuf buf;
	};

	class tcp_ostream : public std::ostream
	{
	public:
		explicit tcp_ostream(const std::pair<int, SSL*>& fd_ssl_pair);

		void SSLUpgrade(SSL* sslPointer);

	private:
		tcp_streambuf buf;
	};

	class tcp_iostream : public std::iostream
	{
	public:
		explicit tcp_iostream(const std::pair<int, SSL*>& fd_ssl_pair);

		void SSLUpgrade(SSL* sslPointer);

	private:
		tcp_streambuf buf;
	};
} // namespace zg
