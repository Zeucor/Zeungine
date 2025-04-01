#pragma once
#include <zg/Standard.hpp>
namespace zg::net
{
    void populate_addr_from_ip(sockaddr_in& addr, const std::string& ip);
}