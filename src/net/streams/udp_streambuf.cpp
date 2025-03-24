#include <zg/net/streams/udp_streambuf.hpp>
using namespace zg::net::streams;
udp_streambuf::udp_streambuf(const SocketPair& fd_addr_pair) :
		fd(std::get<0>(fd_addr_pair)), addr(std::get<1>(fd_addr_pair)), buffer(4196)
{
	setg(buffer.data(), buffer.data(), buffer.data());
	setp(buffer.data(), buffer.data() + buffer.size());
}
udp_streambuf::~udp_streambuf() { sync(); }
void udp_streambuf::setReceivedData(const char* data, size_t length)
{
	auto index = buffer.size();
	buffer.resize(index + length);
	std::copy(data, data + length, buffer.begin() + index);
	setg(buffer.data(), buffer.data() + index, buffer.data() + index + length);
}
int udp_streambuf::underflow()
{
	// if (gptr() < egptr()) // Data is still available
	// 	return traits_type::to_int_type(*gptr());

	// return traits_type::eof();
    if (gptr() < egptr())
        return traits_type::to_int_type(*gptr());

    size_t n = recvfrom(fd, buffer.data(), buffer.size(), 0, (sockaddr*)&addr, &addr_len);
    if (n <= 0)
        return traits_type::eof();

    setg(buffer.data(), buffer.data(), buffer.data() + n);
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
	size_t sent = sendto(fd, pbase(), pptr() - pbase(), 0, (struct sockaddr*)&addr, sizeof(addr));
	if (sent == -1)
		return -1;

	setp(buffer.data(), buffer.data() + buffer.size());
	return 0;
}
