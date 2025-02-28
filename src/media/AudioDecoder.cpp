#include <zg/media/AudioDecoder.hpp>
using namespace zg::media;
AudioDecoder::AudioDecoder(const AVCodec* codec, AVCodecParameters* codecParameters, AVStream* stream) :
		codec(codec), codecParameters(codecParameters), stream(stream)
{
}
size_t AudioDecoder::open() { return 1; }
size_t AudioDecoder::code() { return 1; }
size_t AudioDecoder::flush() { return 0; }
size_t AudioDecoder::close() { return 1; }
