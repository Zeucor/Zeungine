#include <thread>
#include <zg/Logger.hpp>
#include <zg/Serial.hpp>
#include <zg/crypto/Random.hpp>
#include <zg/net/tcp_client.hpp>
#include <zg/net/tcp_server.hpp>
#include <zg/net/udp_client.hpp>
#include <zg/net/udp_server.hpp>
#include <zg/net/ssl_factory.hpp>
#include <condition_variable>
// tcp
#define TCP_PORT 8033
#define TCP_HOST "127.0.0.1"
#define TCP_BITSLENGTH 1024
#define TCP_BITSTREAM true
void tcp_server();
std::vector<bool> serverBits(TCP_BITSLENGTH, false);
std::mutex tcpServerStartMutex;
bool tcpServerStarted = false;
std::condition_variable tcpServerStartCV;
void tcp_client();
// udp
#define UDP_PORT 8091
#define UDP_BITSTREAM true
void udp_server();
std::mutex udpServerStartMutex;
bool udpServerStarted = false;
std::condition_variable udpServerStartCV;
void udp_client();
// main
int main()
{
	std::thread tcpClientThread(tcp_client);
	std::thread tcpServerThread(tcp_server);
	std::thread udpClientThread(udp_client);
	std::thread udpServerThread(udp_server);
	tcpClientThread.join();
	tcpServerThread.join();
	udpClientThread.join();
	udpServerThread.join();
}
void tcp_server()
{
	auto ssl_ctx = zg::net::ssl_factory::createServer();
	zg::net::tcp_server tcpServer(TCP_PORT, TCP_BITSTREAM, ssl_ctx);
	{
		std::unique_lock lock(tcpServerStartMutex);
		tcpServerStarted = true;
	}
	tcpServerStartCV.notify_all();
	// accept a single client (blocking)
	auto& clientTuple = tcpServer.acceptOne();
	// create a serial from the clients stream
	auto& clientSerial = *std::get<1>(clientTuple);
	// write a random hash to the client
	clientSerial.writeBits(serverBits, 0, serverBits.size());
	clientSerial.synchronize();

	zg::Logger::print(zg::Logger::Blank, "TCP_SERVER: synchronized bits");
}
void tcp_client()
{
	{
		std::unique_lock lock(tcpServerStartMutex);
		tcpServerStartCV.wait(lock, [] { return tcpServerStarted; });
	}
	auto ssl_ctx = zg::net::ssl_factory::createClient();
	zg::net::tcp_client tcp_client(TCP_HOST, TCP_PORT, ssl_ctx);
	Serial serial(tcp_client, TCP_BITSTREAM);
	std::vector<bool> clientBits(TCP_BITSLENGTH, false);
	serial.readBits(clientBits, 0, clientBits.size());
	assert(clientBits == serverBits);
	zg::Logger::print(zg::Logger::Blank, "TCP_CLIENT: Received Bits");
}
void udp_server()
{
	zg::net::udp_server udpServer(UDP_PORT, UDP_BITSTREAM);
	{
		std::unique_lock lock(udpServerStartMutex);
		udpServerStarted = true;
	}
	udpServerStartCV.notify_all();
	auto ioStreamPointer = udpServer.receiveOne();
	if (!ioStreamPointer)
	{
		return;
	}
	Serial serial(*ioStreamPointer, UDP_BITSTREAM);
	bool x, y, z, t, u, v, _7;
	serial >> x >> y >> z >> t >> u >> v >> _7;
	zg::Logger::print(zg::Logger::Blank, "UDP_SERVER: RECEIVED BITS: ", x ? "1" : "0", y ? "1" : "0", z ? "1" : "0",
										t ? "1" : "0", u ? "1" : "0", v ? "1" : "0", _7 ? "1" : "0");
}
void udp_client()
{
	{
		std::unique_lock lock(udpServerStartMutex);
		udpServerStartCV.wait(lock, [] { return udpServerStarted; });
	}
	zg::net::udp_client udpClient("127.0.0.1", UDP_PORT);
	Serial serial(udpClient, UDP_BITSTREAM);
	serial << true << true << true << true << true << true << true;
	serial.synchronize();
	zg::Logger::print(zg::Logger::Blank, "UDP_CLIENT: SYNCHRONIZED BITS");
}
