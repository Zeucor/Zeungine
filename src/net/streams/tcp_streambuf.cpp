#include <zg/net/streams/tcp_streambuf.hpp>
using namespace zg::net::streams;
tcp_streambuf::tcp_streambuf(int socket_fd, std::size_t buffer_size) : socket_fd(socket_fd), buffer(buffer_size)
{
    setg(buffer.data(), buffer.data(), buffer.data());
    setp(buffer.data(), buffer.data() + buffer.size());
}
tcp_streambuf::~tcp_streambuf()
{
    sync();
}
int tcp_streambuf::underflow()
{
    if (gptr() < egptr())
        return traits_type::to_int_type(*gptr());

    size_t n = recv(socket_fd, buffer.data(), buffer.size(), 0);
    if (n <= 0)
        return traits_type::eof();

    setg(buffer.data(), buffer.data(), buffer.data() + n);
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
        size_t sent = send(socket_fd, pbase(), n, 0);
        if (sent <= 0)
            return -1;

        pbump(-n);
    }
    return 0;
}