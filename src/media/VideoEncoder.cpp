#include <zg/media/VideoEncoder.hpp>
using namespace zg::media;
VideoEncoder::VideoEncoder(const AVCodec* codec, AVCodecParameters* codecParameters, AVStream* stream) :
		codec(codec), codecParameters(codecParameters), stream(stream)
{
}
size_t VideoEncoder::open() { return 1; }
size_t VideoEncoder::code() { return 1; }
size_t VideoEncoder::flush() { return 0; }
size_t VideoEncoder::close() { return 1; }
