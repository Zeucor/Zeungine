#pragma once
#include <zg/ffmpeg.hpp>
#include <zg/textures/Texture.hpp>
#include "I1xCoder.hpp"
namespace zg::media
{
	struct MediaStream;
	struct ReadMediaStream;
	struct WriteMediaStream;
	struct VideoDecoder : I1xCoder
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
		SwsContext* swsContext = 0;
		AVFrame* rgbaFrame = 0;
		uint8_t* rgbaBuffer = 0;
		uint32_t rgbaBufferSize = 0;
		int rowBytes = 0;
		uint8_t* tempRow = 0;
		uint64_t frameCount = 0;
		long double totalTimeSeconds = 0;
		long double currentTimeSeconds = 0;
		double framesPerSecond = 0;

	public:
		VideoDecoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters, AVStream* stream,
								 const std::shared_ptr<zg::td::queue<AVFrame*>>& frameQueuePointer,
								 const std::shared_ptr<std::mutex>& mutexPointer);

	protected:
		size_t open() override;
		size_t code() override;
		size_t flush() override;
		size_t close() override;

	public:
		bool fillNextFrame(std::shared_ptr<textures::Texture>& texture);
	};
} // namespace zg::media
