#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif
#include <zg/net/populate_addr_from_ip.hpp>
void zg::net::populate_addr_from_ip(sockaddr_in& addr, const std::string& ip)
{
#if defined(__linux__) || defined(MACOS)
	if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0)
#elif defined(_WIN32)
	if (InetPtonA(AF_INET, ip.c_str(), &addr.sin_addr) <= 0)
#endif
	{
		throw std::runtime_error("Invalid address/Address not supported!");
	}
}