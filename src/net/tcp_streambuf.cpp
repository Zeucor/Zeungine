#include <zg/net/tcp_streambuf.hpp>
using namespace zg::net::streams;
tcp_streambuf::tcp_streambuf(const std::pair<int, SSL*>& fd_ssl_pair, std::size_t buffer_size) :
		fd(std::get<0>(fd_ssl_pair)), /*gbuffer(buffer_size), */ pbuffer(buffer_size), ssl(std::get<1>(fd_ssl_pair))
{
	setg(gbuffer.data(), gbuffer.data(), gbuffer.data());
	setp(pbuffer.data(), pbuffer.data() + pbuffer.size());
}
tcp_streambuf::~tcp_streambuf()
{
	sync();
	close();
}
int tcp_streambuf::underflow()
{
	if (gptr() >= egptr())
	{
		gbuffer.resize(gbuffer.size() + readSize);
		setg(gbuffer.data(), gbuffer.data() + readIndex, gbuffer.data() + readIndex + readSize);
	}
	size_t __bytes__read__;
	if (ssl)
		__bytes__read__ = SSL_read(ssl, gptr(), readSize);
	else
		__bytes__read__ = recv(fd, gptr(), readSize, 1);
	if (__bytes__read__ <= 0)
		return traits_type::eof();
	readIndex += __bytes__read__;
	setg(gbuffer.data(), gptr(), gptr() + __bytes__read__);
	return traits_type::to_int_type(*gptr());
}
int tcp_streambuf::overflow(int c)
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
int tcp_streambuf::sync()
{
	size_t n = pptr() - pbase();
	if (n > 0)
	{
		size_t sent;
		if (ssl)
			sent = SSL_write(ssl, pbase(), n);
		else
			sent = send(fd, pbase(), n, 1);
		if (sent <= 0)
			return -1;

		pbump(-n);
	}
	gbuffer.clear();
	readIndex = 0;
	pbuffer.clear();
	return 0;
}
void tcp_streambuf::close()
{
	if (ssl)
	{
		int ret = SSL_shutdown(ssl);
		if (ret == 0)
			SSL_shutdown(ssl);
		SSL_free(ssl);
	}
	if (fd >= 0)
	{
#ifdef _WIN32
		::closesocket(fd);
#else
		::close(fd);
#endif
	}
}
