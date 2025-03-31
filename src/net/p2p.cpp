#include <zg/net/p2p.hpp>
#include <zg/net/http_client.hpp>
using namespace zg::net;
p2p::HostInfo p2p::host_info_factory::create()
{
    HostInfo info;
    auto response = zg::net::http::http_client::restSync("GET", "https://ipinfo.io/loc");
    std::string latlngString(response.body.second.get(), response.body.second.get() + response.body.first);
    if (latlngString.size() && latlngString[latlngString.size() - 1] == '\n')
    {
        latlngString.erase(latlngString.end() - 1);
    }
    auto comPos = latlngString.find(',');
    if (comPos == std::string::npos)
    {
        return info;
    }
    auto latString = latlngString.substr(0, comPos);
    auto lngString = latlngString.substr(comPos + 1, latlngString.size() - 1);
    info.latitude = std::stold(latString);
    info.longitude = std::stold(lngString);
    return info;
}
p2p::p2p(const std::string& announceIP):
    announceIP(announceIP)
    
{
    startAnnounce();
}
bool p2p::doingAnnounce()
{
    return !!announceClient;
}
void p2p::startAnnounce()
{

}
void p2p::closeAnnounce()
{
    return;
}