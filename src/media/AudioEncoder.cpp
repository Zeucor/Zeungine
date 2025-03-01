#include <zg/media/AudioEncoder.hpp>
using namespace zg::media;
AudioEncoder::AudioEncoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters,
													 AVStream* stream) :
		mediaStream(mediaStream), codec(codec), codecParameters(codecParameters), stream(stream)
{
}
size_t AudioEncoder::open() { return 1; }
size_t AudioEncoder::code() { return 1; }
size_t AudioEncoder::flush() { return 0; }
size_t AudioEncoder::close() { return 1; }
