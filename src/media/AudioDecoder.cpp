#include <zg/media/AudioDecoder.hpp>
using namespace zg::media;
AudioDecoder::AudioDecoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters,
													 AVStream* stream) :
		mediaStream(mediaStream), codec(codec), codecParameters(codecParameters), stream(stream)
{
}
size_t AudioDecoder::open()
{
	codecContext = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(codecContext, codecParameters);
	if (avcodec_open2(codecContext, codec, 0) < 0)
		return 0;
	return 1;
}
size_t AudioDecoder::code() { return 1; }
size_t AudioDecoder::flush() { return 0; }
size_t AudioDecoder::close() { return 1; }
