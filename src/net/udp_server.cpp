#include <zg/Logger.hpp>
#include <zg/net/socket_init.hpp>
#include <zg/net/udp_server.hpp>
using namespace zg::net;
#define BACKLOG 5
udp_server::udp_server(int port, bool bitStream) : port(port), bitStream(bitStream)
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
	if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		close();
		throw std::runtime_error("udp_server Bind failed");
	}
}
void udp_server::close()
{
#ifdef _WIN32
	::closesocket(server_fd);
#else
	::close(server_fd);
#endif
}
udp_server::IOStreamPointer udp_server::receiveOne(bool nonBlock, unsigned int nonBlockMicroSecTimeout)
{
	sockaddr_in client_addr;
	SockLength client_len = sizeof(client_addr);
	char buffer[4096];
	memset(buffer, 0, sizeof(buffer));
	if (nonBlock && nonBlockMicroSecTimeout != m_nonBlockMicroSecTimeout)
	{
#if defined(__linux__) || defined(MACOS)
		struct timeval read_timeout;
		read_timeout.tv_sec = 0;
		read_timeout.tv_usec = nonBlockMicroSecTimeout;
#elif defined(_WIN32)
		DWORD read_timeout = nonBlockMicroSecTimeout / 1000;
#endif
		setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&read_timeout, sizeof(read_timeout));
		m_nonBlockMicroSecTimeout = nonBlockMicroSecTimeout;
	}
	//
	//
	size_t recv_len = recvfrom(server_fd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_len);
	if (recv_len == -1)
	{
		throw std::runtime_error("recvfrom failed");
	}
	auto key = std::make_pair(client_addr.sin_addr.s_addr, client_addr.sin_port);
	auto& clientStream = clientStreams[key];
	if (!clientStream)
	{
		clientStream = std::make_shared<zg::net::streams::udp_iostream>(zg::net::streams::udp_streambuf::SocketPair(server_fd, client_addr));
	}
	clientStream->pushData(buffer, recv_len);
	return clientStream;
}
