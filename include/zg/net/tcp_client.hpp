#pragma once
#include "streams/tcp_iostream.hpp"
namespace zg::net
{
	struct tcp_client : zg::net::streams::tcp_iostream
	{
	public:
		tcp_client(const std::string& host, int port, SSL_CTX* ssl_ctx = 0, bool verifyCerts = true);
		~tcp_client();

		void SSLUpgrade(const SSLPair& sslPair);

	private:
		std::pair<int, SSL*> connect(const std::string& host, int port, SSL_CTX* ssl_ctx, bool verifyCerts);
		SSL_CTX* ssl_ctx = 0;
	};
} // namespace zg::net
