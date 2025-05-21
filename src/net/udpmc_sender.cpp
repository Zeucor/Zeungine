#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif
#include <zg/Logger.hpp>
#include <zg/net/socket_init.hpp>
#include <zg/net/udpmc_sender.hpp>
#include <zg/net/resolve_host_or_ip_to_ip.hpp>
#include <zg/net/populate_addr_from_ip.hpp>
using namespace zg::net;
#define BACKLOG 5

void sender_diagnose(bool condition, const char* msg) {
    if (!condition) {
        std::cerr << "DIAGNOSE ERROR: " << msg << " failed. WSAGetLastError: " << WSAGetLastError() << std::endl;
    }
}
udpmc_sender::udpmc_sender(const std::string& host, int port) : udp_ostream(setup(host, port))
{
}
streams::udp_streambuf::SocketPair udpmc_sender::setup(const std::string& host, int port)
{
	socket_init::initialize();
	server_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server_fd == -1)
	{
		throw std::runtime_error("Socket creation failed");
	}
    BOOL reuseAddr = TRUE;
    sender_diagnose(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&reuseAddr, sizeof(reuseAddr)) >= 0, "Setting SO_REUSEADDR");

	sockaddr_in group_addr{};
	group_addr.sin_family = AF_INET;
	auto ip = resolve_host_or_ip_to_ip(host);
	populate_addr_from_ip(group_addr, ip);
	group_addr.sin_port = htons(port);

	in_addr localIface = {};
	localIface.s_addr = htonl(INADDR_ANY);
	sender_diagnose(setsockopt(server_fd, IPPROTO_IP, IP_MULTICAST_IF, (char*)&localIface, sizeof(localIface)) >= 0, "Setting IP_MULTICAST_IF");
	return {server_fd, group_addr};
}
void udpmc_sender::close()
{
#ifdef _WIN32
	::closesocket(server_fd);
#else
	::close(server_fd);
#endif
}
