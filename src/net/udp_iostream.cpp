#include <zg/net/udp_iostream.hpp>
using namespace zg::net::streams;
udp_istream::udp_istream(const udp_streambuf::SocketPair& fd_addr_pair) : std::istream(&buf), buf(fd_addr_pair) {}
udp_ostream::udp_ostream(const udp_streambuf::SocketPair& fd_addr_pair) : std::ostream(&buf), buf(fd_addr_pair) {}
udp_iostream::udp_iostream(const udp_streambuf::SocketPair& fd_addr_pair) : std::iostream(&buf), buf(fd_addr_pair) {}
