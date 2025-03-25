#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif
#include <zg/Logger.hpp>
#include <zg/net/tcp_client.hpp>
using namespace zg::net;
tcp_client::tcp_client(const std::string& host, int port, SSL_CTX* ssl_ctx) : tcp_iostream(connect(host, port, ssl_ctx)) {}
std::pair<int, SSL*> tcp_client::connect(const std::string& host, int port, SSL_CTX* ssl_ctx)
{
	SSL* ssl = 0;
	if (ssl_ctx)
	{
		ssl = SSL_new(ssl_ctx);
	}
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
#if defined(__linux__) || defined(MACOS)
	if (inet_pton(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0)
#elif defined(_WIN32)
	if (InetPtonA(AF_INET, host.c_str(), &server_addr.sin_addr) <= 0)
#endif
	{
		throw std::runtime_error("Invalid address/Address not supported!");
	}
	if (::connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		throw std::runtime_error("Connection failed!");
	}
	if (ssl)
	{
		if (SSL_connect(ssl) < 0)
		{
			throw std::runtime_error("SSL Connection failed!");
		}
	}
	return {sock, ssl};
}
