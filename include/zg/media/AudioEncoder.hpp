#pragma once
#include <zg/ffmpeg.hpp>
#include "I1xCoder.hpp"
namespace zg::media
{
	struct MediaStream;
	struct ReadMediaStream;
	struct WriteMediaStream;
	struct AudioEncoder : I1xCoder
	{
		friend MediaStream;
		friend ReadMediaStream;
		friend WriteMediaStream;

	protected:
		MediaStream& mediaStream;
		const AVCodec* codec = 0;
		AVCodecParameters* codecParameters = 0;
		AVStream* stream = 0;

	public:
		AudioEncoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters, AVStream* stream,
								 const std::shared_ptr<zg::td::queue<AVFrame*>>& frameQueuePointer,
								 const std::shared_ptr<std::mutex>& mutexPointer);

	protected:
		size_t open() override;
		size_t code() override;
		size_t flush() override;
		size_t close() override;
	};
} // namespace zg::media
