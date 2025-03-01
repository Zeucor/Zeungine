#include <zg/media/VideoDecoder.hpp>
using namespace zg::media;
VideoDecoder::VideoDecoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters,
													 AVStream* stream) :
		mediaStream(mediaStream), codec(codec), codecParameters(codecParameters), stream(stream)
{
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
	swsContext = sws_getContext(codecContext->width, codecContext->height, codecContext->pix_fmt,
		codecContext->width, codecContext->height, AV_PIX_FMT_RGBA,
		SWS_BILINEAR, 0, 0, 0);
	rgbaBufferSize = av_image_get_buffer_size(AV_PIX_FMT_RGBA, codecContext->width, codecContext->height, 1);
	rgbaBuffer = (uint8_t *)av_malloc(rgbaBufferSize * sizeof(uint8_t));
    rgbaFrame = av_frame_alloc();
	av_image_fill_arrays(rgbaFrame->data, rgbaFrame->linesize, rgbaBuffer, AV_PIX_FMT_RGBA, codecContext->width, codecContext->height, 1);
	return 1;
}
size_t VideoDecoder::code() { return 1; }
size_t VideoDecoder::flush() { return 0; }
size_t VideoDecoder::close()
{
	av_frame_free(&rgbaFrame);
	return 1;
}
