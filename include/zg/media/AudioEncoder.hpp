#pragma once
#include "I1xCoder.hpp"
namespace zg::media
{
    struct AudioEncoder : I1xCoder
    {
        AudioEncoder();
        size_t code() override;
    };
}