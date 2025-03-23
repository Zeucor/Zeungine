#include <zg/net/streams/client_tcp_iostream.hpp>
using namespace zg::net::streams;
client_tcp_istream::client_tcp_istream(int socket_fd) : std::istream(&buf), buf(socket_fd) {}
client_tcp_ostream::client_tcp_ostream(int socket_fd) : std::ostream(&buf), buf(socket_fd) {}
client_tcp_iostream::client_tcp_iostream(int socket_fd) : std::iostream(&buf), buf(socket_fd) {}