#pragma once
#include <map>
#include <zg/Events.hpp>
#include <zg/Serial.hpp>
#include "streams/udp_iostream.hpp"
namespace zg::net
{
	struct udp_server
	{
	public:
		using IOStreamPointer = std::shared_ptr<zg::net::streams::udp_iostream>;
#if defined(_WIN32)
		using SockLength = int;
#elif defined(__linux__) || defined(MACOS)
		using SockLength = socklen_t;
#endif
	private:
		int port;
		bool bitStream;
		int server_fd = 0;
		unsigned int m_nonBlockMicroSecTimeout = 0;
		std::map<std::pair<uint32_t, uint16_t>, IOStreamPointer> clientStreams;

	public:
		udp_server(int port, bool bitStream = false);
	private:
		void close();
	public:
		/**
		 * @brief call receiveOne inside your IO loop to handle one incoming packet, pass nonBlock = true to return immediately with null pointer if no data available
		 * 
		 * `IOStreamPointer`s are unique to address and client socket, you could use them as keys
		 * 
		 * You can pass *IOStreamPointer through a Serial, or simply (read / write) data (from / to) it
		 * 
		 * However, if reading from an IOStreamPointer you must have already called receiveOne
		 * 
		 * @param nonBlock boolean value indicating whether nonBlocking sockets are enabled. defaults to false (blocking)
		 * @param nonBlockMicroSecTimeout int value for the nonBlocking timeout. defaults to 10 microsecond
		 */
		IOStreamPointer receiveOne(bool nonBlock = false, unsigned int nonBlockMicroSecTimeout = 10);
	};
} // namespace zg::net
