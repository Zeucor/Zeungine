#include <zg/net/udp_iostream.hpp>
using namespace zg::net::streams;
udp_istream::udp_istream(const udp_streambuf::SocketPair& fd_addr_pair) : std::istream(&buf), buf(fd_addr_pair) {}
udp_ostream::udp_ostream(const udp_streambuf::SocketPair& fd_addr_pair) : std::ostream(&buf), buf(fd_addr_pair) {}
void udp_ostream::pushData(const char* data, size_t length) { buf.setReceivedData(data, length); }
udp_iostream::udp_iostream(const udp_streambuf::SocketPair& fd_addr_pair) : std::iostream(&buf), buf(fd_addr_pair) {}
void udp_iostream::pushData(const char* data, size_t length) { buf.setReceivedData(data, length); }
