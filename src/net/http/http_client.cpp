#include <zg/net/http/http_client.hpp>
#include <zg/net/dns/system/system_dns.hpp>
#include <zg/net/ssl_factory.hpp>
#include <zg/net/string_is_ipv4.hpp>
#include <zg/Serial.hpp>
using namespace zg::net::http;
#define PIECE_INDEX_SCHEME      0
#define PIECE_INDEX_SECURE      1
#define PIECE_INDEX_SEPARATOR   2
#define PIECE_INDEX_HOST        3
#define PIECE_INDEX_PORT        4
#define PIECE_INDEX_PATH        5
ExtractedUri http_client::extractUri(const std::string& uri, bool lowercaseHost)
{
    ExtractedUri extracted;
    int pieceIndex = 0;
    bool lowercasePiece = true;
    std::string piece;
    bool firstCharOfPiece = true;
    std::string searchFor;
    std::vector<unsigned char> endWiths;  
    bool appendLastChar;
    bool rollbackAtLastChar = false;
    auto uriSize = uri.size();
    auto uriData = uri.data();
    for (size_t index = 0; index < uriSize; ++index)
    {
        unsigned char ch = uriData[index];
        if (lowercasePiece)
        {
            ch = std::tolower(ch);
        }
        if (firstCharOfPiece)
        {
            switch (pieceIndex)
            {
                case PIECE_INDEX_SCHEME:
                {
                    appendLastChar = true;
                    if(ch == 'h')
                    {
                        searchFor = "http";
                        endWiths.push_back(':');
                    }
                    else
                    {
                        throw std::runtime_error("http scheme not found");
                    }
                    break;
                };
                case PIECE_INDEX_SECURE:
                {
                    appendLastChar = false;
                    rollbackAtLastChar = true;
                    endWiths.push_back(':');
                }
                case PIECE_INDEX_SEPARATOR:
                {
                    appendLastChar = false;
                    searchFor = "://";
                    break;
                };
                case PIECE_INDEX_HOST:
                {
                    appendLastChar = false;
                    endWiths.push_back(':');
                    endWiths.push_back('/');
                    rollbackAtLastChar = true;
                    break;
                };
                case PIECE_INDEX_PORT:
                {
                    appendLastChar = false;
                    endWiths.push_back('/');
                    rollbackAtLastChar = true;
                    break;
                };
                case PIECE_INDEX_PATH:
                {
                    appendLastChar = true;
                    break;
                };
            }
            firstCharOfPiece = false;
        }
        if (std::find(endWiths.begin(), endWiths.end(), ch) != endWiths.end())
        {
            if (appendLastChar)
                piece.insert(piece.end(), ch);
            goto _appendPiece;
        }
        piece.insert(piece.end(), ch);
        if (!searchFor.empty() && piece == searchFor)
        {
            goto _appendPiece;
        }
        continue;
_appendPiece:
        switch (pieceIndex)
        {
            case PIECE_INDEX_SCHEME:
                std::get<INDEX_EXTRACTED_SCHEME>(extracted) = piece;
                break;
            case PIECE_INDEX_SECURE:
                std::get<INDEX_EXTRACTED_SECURE>(extracted) = ("s" == piece);
                break;
            case PIECE_INDEX_HOST:
                std::get<INDEX_EXTRACTED_HOST>(extracted) = piece;
                break;
            case PIECE_INDEX_PORT:
            {
                auto& port = std::get<INDEX_EXTRACTED_PORT>(extracted);
                if (!piece.empty())
                {
                    port = std::stoll(piece);
                }
                else
                {
                    if (std::get<INDEX_EXTRACTED_SCHEME>(extracted) == "http")
                    {
                        if (std::get<INDEX_EXTRACTED_SECURE>(extracted))
                        {
                            port = 443;
                        }
                        else
                        {
                            port = 80;
                        }
                    }
                }
                break;
            }
        }
        pieceIndex++;
        if (pieceIndex == PIECE_INDEX_HOST)
        {
            lowercasePiece = lowercaseHost;
        }
        else if (pieceIndex > PIECE_INDEX_HOST)
        {
            lowercasePiece = false;
        }
        firstCharOfPiece = true;
        appendLastChar = false;
        if (rollbackAtLastChar)
        {
            index--;
        }
        rollbackAtLastChar = false;
        searchFor.clear();
        endWiths.clear();
        piece.clear();
    }
    std::get<INDEX_EXTRACTED_PATH>(extracted) = piece;
    return extracted;
}
http_response http_client::restSync(const Verb& verb, const std::string& uri, const Headers& headers, const Body& body)
{
    auto extracted = extractUri(uri);
    SSL_CTX* ssl_ctx = 0;
    if (std::get<INDEX_EXTRACTED_SECURE>(extracted))
    {
        ssl_ctx = ssl_factory::createClient();
    }
    auto& host = std::get<INDEX_EXTRACTED_HOST>(extracted);
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
    tcp_client tcpClient(ip, std::get<INDEX_EXTRACTED_PORT>(extracted), ssl_ctx, false);
    Serial serial(tcpClient);
    http_request request;
    request.verb = verb;
    request.path = std::get<4>(extracted);
    request.host = std::get<2>(extracted);
    request.headers = headers;
    request.body = body;
    (serial << request).synchronize();
    http_response response;
    serial >> response;
    return response;
}
zg::UniqueIdentifier http_client::restAsync(const std::function<void(const http_response&)>& callback, const Verb& verb, const std::string& uri, const Headers& headers, const Body& body)
{
    auto id = ++std::get<0>(tcpClients);
    auto& clientsMap = std::get<1>(tcpClients);
    ClientTuple clientTuple;
    clientsMap[id] = clientTuple;
    // {{}, [verb, uri, headers, body]()->http_response
    // {
    //     http_response response;
    //     return response;
    // },
    // []()
    // {
    //     auto& clientsMap = std::get<1>(tcpClients);
    //     // tcpClients
    // }};
    // auto extracted = extractUri(uri);
    // SSL_CTX* ssl_ctx = 0;
    // if (std::get<INDEX_EXTRACTED_SECURE>(extracted))
    // {
    //     ssl_ctx = ssl_factory::createClient();
    // }
    // //
    // clientsMap[id] = std::make_shared<tcp_client>(std::get<INDEX_EXTRACTED_HOST>(extracted), std::get<INDEX_EXTRACTED_PORT>(extracted), ssl_ctx, false);
    return id;
}
bool http_client::cancelRestAsync(UniqueIdentifier& id)
{
    auto& clientsMap = std::get<1>(tcpClients);
    auto iter = clientsMap.find(id);
    if (iter == clientsMap.end())
    {
        return false;
    }
    clientsMap.erase(iter);
    id = 0;
    return true;
}