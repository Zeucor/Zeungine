#include <zg/Logger.hpp>
#include <zg/net/socket_init.hpp>
#include <zg/net/udpmc_sender.hpp>
#include <zg/net/resolve_host_or_ip_to_ip.hpp>
#include <zg/net/populate_addr_from_ip.hpp>
using namespace zg::net;
#define BACKLOG 5
udpmc_sender::udpmc_sender(const std::string& host, int port) : udp_ostream(setup(host, port))
{
}
streams::udp_streambuf::SocketPair udpmc_sender::setup(const std::string& host, int port)
{
	socket_init::initialize();
	server_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (server_fd == -1)
	{
		throw std::runtime_error("Socket creation failed");
	}
	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);
	auto ip = resolve_host_or_ip_to_ip(host);
	populate_addr_from_ip(server_addr, ip);
	return {server_fd, server_addr};
}
void udpmc_sender::close()
{
#ifdef _WIN32
	::closesocket(server_fd);
#else
	::close(server_fd);
#endif
}
