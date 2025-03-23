#include <zg/net/streams/udp_iostream.hpp>
using namespace zg::net::streams;
udp_istream::udp_istream(int server_fd, sockaddr_in addr) : std::istream(&buf), buf(server_fd, addr) {}
udp_ostream::udp_ostream(int server_fd, sockaddr_in addr) : std::ostream(&buf), buf(server_fd, addr) {}
void udp_ostream::pushData(const char* data, size_t length) { buf.setReceivedData(data, length); }
udp_iostream::udp_iostream(int server_fd, sockaddr_in addr) : std::iostream(&buf), buf(server_fd, addr) {}
void udp_iostream::pushData(const char* data, size_t length) { buf.setReceivedData(data, length); }
