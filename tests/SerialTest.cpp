#include <thread>
#include <zg/Logger.hpp>
#include <zg/Serial.hpp>
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif
#include <zg/net/streams/client_tcp_iostream.hpp>
#include <zg/net/tcp_server.hpp>
void server();
std::shared_ptr<zg::net::tcp_server> tcpServer;
#define PORT 8080
#define BITSTREAM true
void client();
int main()
{
	tcpServer = std::make_shared<zg::net::tcp_server>(PORT, BITSTREAM);
	std::thread clientThread(client);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	std::thread serverThread(server);
	clientThread.join();
	serverThread.join();
}
void server()
{
	auto& clientTuple = tcpServer->acceptOne();
	auto& clientSerial = *std::get<1>(clientTuple);
	bool bitCon[7] = {false};
	for (auto e = 0; e < 7; e++)
	{
		clientSerial >> bitCon[e];
		zg::Logger::print(zg::Logger::Blank, "SERVER: received bit: ", (bitCon[e] ? "true" : "false"));
		clientSerial << bitCon[e];
		zg::Logger::print(zg::Logger::Blank, "SERVER: sent bit: ", (bitCon[e] ? "true" : "false"));
	}
	clientSerial.synchronize();
	zg::Logger::print(zg::Logger::Blank, "SERVER: synchronized bits");
}
void client()
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in server_addr{};
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
#if defined(__linux__) || defined(MACOS)
	if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
#elif defined(_WIN32)
	if (InetPtonA(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0)
#endif
	{
		zg::Logger::print(zg::Logger::Error, "Invalid address/Address not supported!");
		return;
	}
	if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		zg::Logger::print(zg::Logger::Error, "Connection failed!");
		return;
	}
	zg::net::streams::client_tcp_iostream tcp_in_out(sock);
	Serial serial(tcp_in_out, BITSTREAM);
	zg::Logger::print(zg::Logger::Blank, "CLIENT: Sending Bits");
	(serial << true << true << true << false << true << true << true).synchronize();
	zg::Logger::print(zg::Logger::Blank, "CLIENT: Sent Bits");
	bool bitCon[7] = {false};
	for (auto e = 0; e < 7; e++)
		serial >> bitCon[e];
	zg::Logger::print(zg::Logger::Blank, "CLIENT: Received Bits");
}
