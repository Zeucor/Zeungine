#pragma once
#include <zg/Standard.hpp>
namespace zg::media
{
    struct I1xCoder
    {
        virtual ~I1xCoder() = default;
        virtual size_t open() { return 1; };
        virtual size_t code() { return 1; };
        virtual size_t close() { return 1; };
    };
}