#pragma once
#include <zg/Standard.hpp>
namespace zg::net::http
{
    using Scheme = std::string;
    using Secure = bool;
    using Host = std::string;
    using Port = int;
    using Verb = std::string;
    using Path = std::string;
    using Headers = std::unordered_map<std::string, std::string>;
    using Body = std::pair<size_t, std::shared_ptr<int8_t>>;
    #define INDEX_EXTRACTED_SCHEME  0
    #define INDEX_EXTRACTED_SECURE  1
    #define INDEX_EXTRACTED_HOST    2
    #define INDEX_EXTRACTED_PORT    3
    #define INDEX_EXTRACTED_PATH    4
    using ExtractedUri = std::tuple<Scheme, Secure, Host, Port, Path>;
    struct http_request
    {
        Host host;
        Verb verb;
        Path path;
        Headers headers;
        Body body;
    };
    struct http_response
    {
        std::string protocol;
        std::string version;
        std::string statusCode;
        std::string statusText;
        Headers headers;
        Body body;
    };
}