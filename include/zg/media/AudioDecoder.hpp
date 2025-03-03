#pragma once
#include <zg/ffmpeg.hpp>
#include <zg/queue.hpp>
#include "I1xCoder.hpp"
namespace zg::media
{
	struct MediaStream;
	struct ReadMediaStream;
	struct WriteMediaStream;
	struct AudioDecoder : I1xCoder
	{
		friend MediaStream;
		friend ReadMediaStream;
		friend WriteMediaStream;

	protected:
		MediaStream& mediaStream;
		const AVCodec* codec = 0;
		AVCodecParameters* codecParameters = 0;
		AVStream* stream = 0;
		AVCodecContext* codecContext = 0;
		SwrContext* swrContext = 0;
		AVFrame* audioFrame = 0;
		zg::td::queue<float> sampleQueue;

	public:
		AudioDecoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters, AVStream* stream,
								 const std::shared_ptr<zg::td::queue<AVFrame*>>& frameQueuePointer,
								 const std::shared_ptr<std::mutex>& mutexPointer);

	private:
		size_t open() override;
		size_t code() override;
		size_t flush() override;
		size_t close() override;

	protected:
		AVChannelLayout MAToAV_ChannelLayout();
		AVSampleFormat MAToAV_SampleFormat();
		void fillNextFrames(float* frames, const int32_t& channelCount, const unsigned long& frameCount);
	};
} // namespace zg::media
