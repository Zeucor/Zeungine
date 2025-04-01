#include <zg/Logger.hpp>
#include <zg/net/udpmc_receiver.hpp>
#include <zg/net/resolve_host_or_ip_to_ip.hpp>
#include <zg/net/populate_addr_from_ip.hpp>
#include <zg/net/socket_init.hpp>
using namespace zg::net;
udpmc_receiver::udpmc_receiver(const std::string& host, int port) : udp_istream(bind(host, port))
{
	buf.readSize = 1536;
	memset(&buf.addr, 0, sizeof(buf.addr));
}
streams::udp_streambuf::SocketPair udpmc_receiver::bind(const std::string& host, int port)
{
	socket_init::initialize();
	streams::udp_streambuf::SocketIdentifier sock = socket(AF_INET, SOCK_DGRAM, 0);
	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	auto ip = resolve_host_or_ip_to_ip(host);
	populate_addr_from_ip(server_addr, ip);
    ::bind(sock, (sockaddr*)&server_addr, sizeof(server_addr));
    ip_mreq mreq = {};
    mreq.imr_multiaddr.s_addr = inet_addr(ip.c_str());
    mreq.imr_interface.s_addr = INADDR_ANY;
    setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq));
	return {sock, server_addr};
}
