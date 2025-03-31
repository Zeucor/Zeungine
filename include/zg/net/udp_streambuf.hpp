#pragma once
#include <streambuf>
#include <vector>
#include <zg/Standard.hpp>
namespace zg::net::streams
{
	class udp_streambuf : public std::streambuf
	{
	public:
#if defined(_WIN32)
		using SocketIdentifier = SOCKET;
#elif defined(__linux__) || defined(MACOS)
		using SocketIdentifier = int;
#endif
		using SocketPair = std::pair<SocketIdentifier, sockaddr_in>;
		explicit udp_streambuf(const SocketPair& fd_addr_pair);

		~udp_streambuf();

		void setReceivedData(const char* data, size_t length);

	protected:
		int underflow() override;

		int overflow(int c = traits_type::eof()) override;

		int sync() override;

	private:
		SocketIdentifier fd;
		sockaddr_in addr;
		std::vector<char> buffer;
#ifdef _WIN32
		inline static int addr_len = sizeof(sockaddr_in);
#else
		inline static socklen_t addr_len = sizeof(sockaddr_in);
#endif
	};
} // namespace zg::net::streams
