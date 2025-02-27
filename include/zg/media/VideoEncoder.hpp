#pragma once
#include "I1xCoder.hpp"
namespace zg::media
{
    struct VideoEncoder : I1xCoder
    {
        VideoEncoder();
        size_t code() override;
    };
}