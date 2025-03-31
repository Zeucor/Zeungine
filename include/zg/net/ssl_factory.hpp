#pragma once
#include <zg/interfaces/IFile.hpp>
#include <openssl/ssl.h>
namespace zg::net
{
    struct ssl_factory
    {
        static SSL_CTX* createClient(const std::string& host);
        static SSL_CTX* createServer();
        static SSL_CTX* createServer(const interfaces::IFile &cert, const interfaces::IFile &key);
        static SSL_CTX* createServer(const std::string &cert, const std::string &key);
    };
}