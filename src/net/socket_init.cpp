#include <zg/net/socket_init.hpp>
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // Link against Winsock library
#endif
#include <zg/Logger.hpp>
using namespace zg::net;
bool socket_init::initialized =
#ifdef _WIN32
	false;
#else
	true;
#endif
void socket_init::initialize()
{
	if (initialized)
		return;
#ifdef _WIN32
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		throw std::runtime_error("WSAStartup failed");
	}
#endif
	initialized = true;
}
