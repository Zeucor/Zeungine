#pragma once
#include <zg/ffmpeg.hpp>
#include "I1xCoder.hpp"
#include <zg/queue.hpp>
namespace zg::media
{
	struct MediaStream;
	struct AudioDecoder : I1xCoder
	{
		MediaStream& mediaStream;
		const AVCodec* codec = 0;
		AVCodecParameters* codecParameters = 0;
		AVStream* stream = 0;
		AVCodecContext* codecContext = 0;
		SwrContext* swrContext = 0;
		AVFrame* audioFrame = 0;
		zg::td::queue<float> sampleQueue;
		AudioDecoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters, AVStream* stream);
		size_t open() override;
		size_t code() override;
		size_t flush() override;
		size_t close() override;
		AVChannelLayout MAToAV_ChannelLayout();
		AVSampleFormat MAToAV_SampleFormat();
	};
} // namespace zg::media
