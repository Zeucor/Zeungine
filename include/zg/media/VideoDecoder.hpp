#pragma once
#include <zg/ffmpeg.hpp>
#include "I1xCoder.hpp"
namespace zg::media
{
	struct MediaStream;
	struct VideoDecoder : I1xCoder
	{
		MediaStream& mediaStream;
		const AVCodec* codec = 0;
		AVCodecParameters* codecParameters = 0;
		AVStream* stream = 0;
		AVCodecContext* codecContext = 0;
		SwsContext* swsContext = 0;
		AVFrame* rgbaFrame = 0;
		uint8_t *rgbaBuffer = 0;
		uint32_t rgbaBufferSize = 0;
		VideoDecoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters, AVStream* stream, const std::shared_ptr<zg::td::queue<AVFrame*>>& frameQueuePointer, const std::shared_ptr<std::mutex>& mutexPointer);
		size_t open() override;
		size_t code() override;
		size_t flush() override;
		size_t close() override;
	};
} // namespace zg::media
