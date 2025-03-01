#include <zg/media/VideoDecoder.hpp>
using namespace zg::media;
VideoDecoder::VideoDecoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters,
													 AVStream* stream) :
		mediaStream(mediaStream), codec(codec), codecParameters(codecParameters), stream(stream)
{
}
size_t VideoDecoder::open() { return 1; }
size_t VideoDecoder::code() { return 1; }
size_t VideoDecoder::flush() { return 0; }
size_t VideoDecoder::close() { return 1; }
