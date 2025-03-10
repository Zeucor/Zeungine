#include <zg/media/VideoEncoder.hpp>
using namespace zg::media;
VideoEncoder::VideoEncoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters,
													 AVStream* stream, const std::shared_ptr<zg::td::queue<AVFrame*>>& frameQueuePointer,
													 const std::shared_ptr<std::mutex>& mutexPointer) :
		I1xCoder(frameQueuePointer, mutexPointer), mediaStream(mediaStream), codec(codec), codecParameters(codecParameters),
		stream(stream)
{
	open();
}
size_t VideoEncoder::open() { return 1; }
size_t VideoEncoder::code() { return 1; }
size_t VideoEncoder::flush() { return 0; }
size_t VideoEncoder::close() { return 1; }
