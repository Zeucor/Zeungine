#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif
#include <zg/Logger.hpp>
#include <zg/net/tcp_client.hpp>
#include <zg/net/socket_init.hpp>
#include <zg/net/string_is_ipv4.hpp>
#include <zg/net/dns/system/system_dns.hpp>
using namespace zg::net;
tcp_client::tcp_client(const std::string& host, int port, SSL_CTX* ssl_ctx, bool verifyCerts) : tcp_iostream(connect(host, port, ssl_ctx, verifyCerts)),
	ssl_ctx(ssl_ctx) {}
tcp_client::~tcp_client()
{
    if (ssl_ctx)
    {
        SSL_CTX_free(ssl_ctx);
    }
}
std::pair<int, SSL*> tcp_client::connect(const std::string& host, int port, SSL_CTX* ssl_ctx, bool verifyCerts)
{
    std::string ip;
    if (zg::net::string_is_ipv4(host))
    {
        ip = host;
    }
    else
    {
        auto ips = zg::net::dns::system::system_dns::queryA(host);
        if (ips.size())
        {
            ip = ips[0];
        }
        else
        {
            throw std::runtime_error("Could not find ip for host: " + host);
        }
    }
	socket_init::initialize();
	SSL* ssl = 0;
	if (ssl_ctx)
	{
		ssl = SSL_new(ssl_ctx);
		if (!verifyCerts)
			SSL_set_verify(ssl, SSL_VERIFY_NONE, NULL);
	}
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
#if defined(__linux__) || defined(MACOS)
	if (inet_pton(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0)
#elif defined(_WIN32)
	if (InetPtonA(AF_INET, ip.c_str(), &server_addr.sin_addr) <= 0)
#endif
	{
		throw std::runtime_error("Invalid address/Address not supported!");
	}
	if (::connect(fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		throw std::runtime_error("Connection failed!");
	}
	if (ssl)
	{
		SSL_set_fd(ssl, fd);
		SSL_set_tlsext_host_name(ssl, host.c_str());
		int ret = 0;
		if ((ret = SSL_connect(ssl)) < 0)
		{
			int ssl_err = SSL_get_error(ssl, ret);
			unsigned long err = 0;
			std::string errStr;
			while ((err = ERR_get_error()) != 0)
			{
				errStr += "OpenSSL Error: " + std::string(ERR_reason_error_string(err)) + "\n";
			}
			throw std::runtime_error("SSL Connection failed: " + errStr);
		}
	}
	return {fd, ssl};
}
