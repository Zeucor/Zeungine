#include <zg/media/AudioEncoder.hpp>
using namespace zg::media;
AudioEncoder::AudioEncoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters,
													 AVStream* stream, const std::shared_ptr<zg::td::queue<AVFrame*>>& frameQueuePointer,
													 const std::shared_ptr<std::mutex>& mutexPointer) :
		I1xCoder(frameQueuePointer, mutexPointer), mediaStream(mediaStream), codec(codec), codecParameters(codecParameters),
		stream(stream)
{
}
size_t AudioEncoder::open() { return 1; }
size_t AudioEncoder::code() { return 1; }
size_t AudioEncoder::flush() { return 0; }
size_t AudioEncoder::close() { return 1; }
