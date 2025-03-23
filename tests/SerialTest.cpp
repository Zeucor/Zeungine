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
#include <zg/crypto/Random.hpp>
void server();
std::shared_ptr<zg::net::tcp_server> tcpServer;
#define PORT 8080
#define BITSTREAM true
#define BITSLENGTH 1024
std::vector<bool> clientBits(BITSLENGTH);
std::vector<bool> serverBits(BITSLENGTH);
void client();
int main()
{
	tcpServer = std::make_shared<zg::net::tcp_server>(PORT, BITSTREAM);
	auto serverBitsSize = serverBits.size();
	for (auto i = 0; i < serverBitsSize; ++i)
	{
		serverBits[i] = zg::crypto::Random::value<short>(0, 1);
	}
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
	clientSerial.writeBits(serverBits, 0, serverBits.size());
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
	serial.readBits(clientBits, 0, clientBits.size());
	for (const auto &bit : clientBits)
	{
		zg::Logger::print(zg::Logger::Blank, "Client Bit: ", bit ? "1" : "0");
	}
	assert(clientBits == serverBits);
	zg::Logger::print(zg::Logger::Blank, "CLIENT: Received Bits");
}
