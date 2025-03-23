#pragma once
namespace zg::net
{
    struct socket_init
    {
        static bool initialized;
        static void initialize();
    };
}