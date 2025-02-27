#pragma once
#include <zg/Standard.hpp>
namespace zg::media
{
    struct I1xCoder
    {
        virtual ~I1xCoder() = default;
        virtual size_t code() = 0;
    };
}