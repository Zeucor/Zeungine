#include <zg/media/MediaStream.hpp>
#include <zg/media/VideoDecoder.hpp>
using namespace zg::media;
VideoDecoder::VideoDecoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters,
													 AVStream* stream, const std::shared_ptr<zg::td::queue<AVFrame*>>& frameQueuePointer,
													 const std::shared_ptr<std::mutex>& mutexPointer) :
		I1xCoder(frameQueuePointer, mutexPointer), mediaStream(mediaStream), codec(codec), codecParameters(codecParameters),
		stream(stream)
{
	open();
}
size_t VideoDecoder::open()
{
	avcodec_find_decoder(codecParameters->codec_id);
	codecContext = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(codecContext, codecParameters);
	if (codecContext->pix_fmt == AV_PIX_FMT_YUVJ444P)
	{
		codecContext->color_range = AVCOL_RANGE_JPEG;
		codecContext->pix_fmt = AV_PIX_FMT_YUV444P;
	}
	if (avcodec_open2(codecContext, codec, 0) < 0)
	{
		std::cerr << "avcodec_open2 error" << std::endl;
		return 0;
	}
	swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt, codecContext->width,
															codecContext->height, AV_PIX_FMT_RGBA, SWS_BILINEAR, 0, 0, 0);
															rgbaBufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, codecContext->width, codecContext->height, 1);
	rgbaBuffer = (uint8_t*)av_malloc(rgbaBufferSize * sizeof(uint8_t));
	rgbaFrame = av_frame_alloc();
	av_image_fill_arrays(rgbaFrame->data, rgbaFrame->linesize, rgbaBuffer, AV_PIX_FMT_RGBA, codecContext->width,
											 codecContext->height, 1);
	static constexpr int bytesPerPixel = 4;
	rowBytes = codecContext->width * bytesPerPixel;
	// tempRow = new uint8_t[rowBytes];
	AVRational frameRate = av_guess_frame_rate(mediaStream.formatContext, stream, 0);
	framesPerSecond = frameRate.num / static_cast<double>(frameRate.den);
	int64_t duration = mediaStream.formatContext->duration;
	totalTimeSeconds = (double)duration / AV_TIME_BASE;
	frameCount = totalTimeSeconds * framesPerSecond;
	return 1;
}
size_t VideoDecoder::code() { return 1; }
size_t VideoDecoder::flush() { return 0; }
size_t VideoDecoder::close()
{
	av_frame_free(&rgbaFrame);
	// delete[] tempRow;
	return 1;
}
bool VideoDecoder::fillNextFrame(std::shared_ptr<textures::Texture>& texturePointer)
{
	auto& frameQueueRef = *frameQueuePointer;
	AVFrame* currentAVFrame = 0;
	{
		std::lock_guard lock(*mutexPointer);
		if (frameQueueRef.empty())
			return false;
		currentAVFrame = frameQueueRef.front();
		frameQueueRef.pop();
	}
	sws_scale(swsContext, (uint8_t const* const*)currentAVFrame->data, currentAVFrame->linesize, 0, codecContext->height,
						rgbaFrame->data, rgbaFrame->linesize);
	// for (int i = 0; i < codecContext->height / 2; ++i)
	// {
	// 	uint8_t* rowTop = rgbaBuffer + i * rowBytes;
	// 	uint8_t* rowBottom = rgbaBuffer + (codecContext->height - i - 1) * rowBytes;
	// 	memcpy(tempRow, rowTop, rowBytes);
	// 	memcpy(rowTop, rowBottom, rowBytes);
	// 	memcpy(rowBottom, tempRow, rowBytes);
	// }
	texturePointer = std::make_shared<textures::Texture>(
		mediaStream.streamWindow, glm::ivec4(codecContext->width, codecContext->height, 1, 0), (void*)rgbaBuffer);
	return true;
}
