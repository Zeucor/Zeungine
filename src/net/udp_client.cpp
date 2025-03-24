#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif
#include <zg/Logger.hpp>
#include <zg/net/udp_client.hpp>
using namespace zg::net;
udp_client::udp_client(const std::string& host, int port) : udp_iostream(connect(host, port)) {}
streams::udp_streambuf::SocketPair udp_client::connect(const std::string& host, int port)
{
	streams::udp_streambuf::SocketIdentifier sock = socket(AF_INET, SOCK_DGRAM, 0);
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
	return {sock, server_addr};
}
