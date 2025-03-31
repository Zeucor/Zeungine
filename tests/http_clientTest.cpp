#include <zg/net/http/http_client.hpp>
#include <zg/Logger.hpp>
int main()
{
    auto response = zg::net::http::http_client::restSync("GET", "https://search.brave.com/");
    zg::Logger::print(
        zg::Logger::Blank,
        "StatusCode: ",
        response.statusCode,
        "\nStatus Text: ",
        response.statusText,
        "\nProtocol: ",
        response.protocol,
        "\nVersion: ",
        response.version,
        "\nContent Length: ",
        response.body.first,
        "\nBody: ",
        response.body.second.get());
}