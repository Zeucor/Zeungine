#include <zg/net/streams/tcp_iostream.hpp>
using namespace zg::net::streams;
tcp_istream::tcp_istream(int socket_fd) : std::istream(&buf), buf(socket_fd) {}
tcp_ostream::tcp_ostream(int socket_fd) : std::ostream(&buf), buf(socket_fd) {}
tcp_iostream::tcp_iostream(int socket_fd) : std::iostream(&buf), buf(socket_fd) {}