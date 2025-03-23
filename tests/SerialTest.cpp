#include <thread>
#include <zg/Logger.hpp>
#include <zg/Serial.hpp>
#include <zg/net/tcp_client.hpp>
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
	zg::net::tcp_client tcp_client("127.0.0.1", 8080);
	Serial serial(tcp_client, BITSTREAM);
	serial.readBits(clientBits, 0, clientBits.size());
	for (const auto &bit : clientBits)
	{
		zg::Logger::print(zg::Logger::Blank, "Client Bit: ", bit ? "1" : "0");
	}
	assert(clientBits == serverBits);
	zg::Logger::print(zg::Logger::Blank, "CLIENT: Received Bits");
}
