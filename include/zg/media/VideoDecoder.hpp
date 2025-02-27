#pragma once
#include "I1xCoder.hpp"
namespace zg::media
{
    struct VideoDecoder : I1xCoder
    {
        VideoDecoder();
        size_t code() override;
    };
}