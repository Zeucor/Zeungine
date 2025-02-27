#pragma once
#include "I1xCoder.hpp"
namespace zg::media
{
    struct AudioDecoder : I1xCoder
    {
        AudioDecoder();
        size_t code() override;
    };
}