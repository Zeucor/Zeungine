#pragma once
#include "streams/tcp_iostream.hpp"
namespace zg::net
{
	struct tcp_client : zg::net::streams::tcp_iostream
	{
	public:
		tcp_client(const std::string& host, int port, SSL_CTX* ssl_ctx = 0);

		void SSLUpgrade(const SSLPair& sslPair);

	private:
		std::pair<int, SSL*> connect(const std::string& host, int port, SSL_CTX* ssl_ctx);
		SSL* m_ssl;
	};
} // namespace zg::net
