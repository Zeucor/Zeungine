#include <zg/net/tcp_iostream.hpp>
using namespace zg::net::streams;
tcp_istream::tcp_istream(const std::pair<int, SSL*>& fd_ssl_pair) : std::istream(&buf), buf(fd_ssl_pair) {}
tcp_ostream::tcp_ostream(const std::pair<int, SSL*>& fd_ssl_pair) : std::ostream(&buf), buf(fd_ssl_pair) {}
tcp_iostream::tcp_iostream(const std::pair<int, SSL*>& fd_ssl_pair) : std::iostream(&buf), buf(fd_ssl_pair) {}