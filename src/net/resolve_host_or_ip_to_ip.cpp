#include <zg/net/resolve_host_or_ip_to_ip.hpp>
#include <zg/net/string_is_ipv4.hpp>
#include <zg/net/system_dns.hpp>
#include <stdexcept>
std::string zg::net::resolve_host_or_ip_to_ip(const std::string& host)
{
    std::string ip;
    if (zg::net::string_is_ipv4(host))
    {
        ip = host;
    }
    else
    {
        auto ips = zg::net::dns::system::system_dns::queryA(host);
        if (ips.size())
        {
            ip = ips[0];
        }
        else
        {
            throw std::runtime_error("Could not find ip for host: " + host);
        }
    }
    return ip;
}