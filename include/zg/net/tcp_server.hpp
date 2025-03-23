#pragma once
#include <map>
#include <zg/Events.hpp>
#include <zg/Serial.hpp>
#include "streams/client_tcp_iostream.hpp"
namespace zg::net
{
	struct tcp_server
	{
	public:
		using ClientTuple =
			std::tuple<int, std::shared_ptr<Serial>, std::shared_ptr<zg::net::streams::client_tcp_iostream>>;
#if defined(_WIN32)
		using SockLength = int;
#elif defined(__linux__) || defined(MACOS)
		using SockLength = socklen_t;
#endif
	private:
		int port;
		bool bitStream;
		int server_fd = 0;
		UniqueIdentifier totalClients;
		std::map<UniqueIdentifier, ClientTuple> clientStreamMap;

	public:
		tcp_server(int port, bool bitStream = false);
		void close();
		ClientTuple& acceptOne();
	};
} // namespace zg::net
