#pragma once
#include "I1xCoder.hpp"
#include <zg/ffmpeg.hpp>
namespace zg::media
{
    struct AudioDecoder : I1xCoder
    {
        const AVCodec* codec = 0;
        AVCodecParameters* codecParameters = 0;
        AVStream* stream = 0;
        AudioDecoder(const AVCodec* codec, AVCodecParameters* codecParameters, AVStream* stream);
        size_t open() override;
        size_t code() override;
        size_t flush() override;
        size_t close() override;
    };
}