#pragma once
#include "http_common.hpp"
#include "../tcp_client.hpp"
#include <zg/Events.hpp>
#include <functional>
namespace zg::net::http
{
    struct http_client
    {
        using ClientTuple = std::tuple<std::shared_ptr<tcp_client>, std::function<http_response()>, std::shared_ptr<std::thread>>;
        inline static std::pair<UniqueIdentifier, std::map<UniqueIdentifier, ClientTuple>> tcpClients = {0, {}};
        static ExtractedUri extractUri(const std::string& uri, bool lowercaseHost = false);
        static http_response restSync(const Verb& verb, const std::string& uri, const Headers& headers = {}, const Body& body = {});
        static UniqueIdentifier restAsync(const std::function<void(const http_response&)>& callback, const Verb& verb, const std::string& uri, const Headers& headers = {}, const Body& body = {});
        static bool cancelRestAsync(UniqueIdentifier& id);
    };
}