#include <zg/net/udp_streambuf.hpp>
#include <zg/Logger.hpp>
#include <cerrno>
using namespace zg::net::streams;
udp_streambuf::udp_streambuf(const SocketPair& fd_addr_pair) :
		fd(std::get<0>(fd_addr_pair)), addr(std::get<1>(fd_addr_pair)), pbuffer(4196)
{
	setg(gbuffer.data(), gbuffer.data(), gbuffer.data());
	setp(pbuffer.data(), pbuffer.data() + pbuffer.size());
}
udp_streambuf::~udp_streambuf()
{
	sync();
	close();
}
int udp_streambuf::underflow()
{
	if (gptr() >= egptr())
	{
		gbuffer.resize(gbuffer.size() + readSize);
		setg(gbuffer.data(), gbuffer.data() + readIndex, gbuffer.data() + readIndex + readSize);
	}
	long long __bytes__read__ = recvfrom(fd, gptr(), readSize, 1, (sockaddr*)&addr, &addr_len);
	if (__bytes__read__ <= 0)
	{
		zg::Logger::print(zg::Logger::Error, "recvfom_failed! " + std::string(strerror(errno)));
	}
	readIndex += __bytes__read__;
	setg(gbuffer.data(), gptr(), gptr() + __bytes__read__);
	return traits_type::to_int_type(*gptr());
}
int udp_streambuf::overflow(int c)
{
	if (sync() == -1)
		return traits_type::eof();

	if (c != traits_type::eof())
	{
		*pptr() = c;
		pbump(1);
	}
	return c;
}
int udp_streambuf::sync()
{
	size_t n = pptr() - pbase();
	if (n > 0)
	{
		size_t sent = sendto(fd, pbase(), pptr() - pbase(), 1, (struct sockaddr*)&addr, sizeof(addr));
		if (sent <= 0)
			return -1;

		pbump(-n);
	}
	gbuffer.clear();
	readIndex = 0;
	pbuffer.clear();
	return 0;
}
void udp_streambuf::close()
{
	if (fd >= 0)
	{
#ifdef _WIN32
		::closesocket(fd);
#else
		::close(fd);
#endif
	}
}
