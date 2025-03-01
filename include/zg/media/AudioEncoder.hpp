#pragma once
#include <zg/ffmpeg.hpp>
#include "I1xCoder.hpp"
namespace zg::media
{
	struct MediaStream;
	struct AudioEncoder : I1xCoder
	{
		MediaStream& mediaStream;
		const AVCodec* codec = 0;
		AVCodecParameters* codecParameters = 0;
		AVStream* stream = 0;
		AudioEncoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters, AVStream* stream, const std::shared_ptr<zg::td::queue<AVFrame*>>& frameQueuePointer, const std::shared_ptr<std::mutex>& mutexPointer);
		size_t open() override;
		size_t code() override;
		size_t flush() override;
		size_t close() override;
	};
} // namespace zg::media
