#include <zg/net/p2p.hpp>
using namespace zg::net;
p2p::HostInfo p2p::host_info_factory::create()
{
    HostInfo info;
    return info;
}
p2p::p2p(const std::string& announceIP):
    announceIP(announceIP)
    
{
    startAnnounce();
}