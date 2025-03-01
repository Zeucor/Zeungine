#include <zg/media/AudioDecoder.hpp>
#include <zg/media/MediaStream.hpp>
using namespace zg::media;
AudioDecoder::AudioDecoder(MediaStream& mediaStream, const AVCodec* codec, AVCodecParameters* codecParameters,
													 AVStream* stream) :
		mediaStream(mediaStream), codec(codec), codecParameters(codecParameters), stream(stream)
{
}
size_t AudioDecoder::open()
{
	codecContext = avcodec_alloc_context3(codec);
	avcodec_parameters_to_context(codecContext, codecParameters);
	if (avcodec_open2(codecContext, codec, 0) < 0)
		return 0;
	auto AV_CH = MAToAV_ChannelLayout();
	auto AV_SAMPLE_FMT = MAToAV_SampleFormat();
	auto& sampleRate = mediaStream.window.audioEngine.sampleRate;
	swr_alloc_set_opts2(&swrContext,
		&AV_CH, AV_SAMPLE_FMT,
		sampleRate,
		&codecContext->ch_layout,
		codecContext->sample_fmt,
		codecContext->sample_rate,
		0, 0);
	if (!swrContext || swr_init(swrContext) < 0)
		return 0;
	audioFrame = av_frame_alloc();
	return 1;
}
size_t AudioDecoder::code() { return 1; }
size_t AudioDecoder::flush() { return 0; }
size_t AudioDecoder::close()
{
	if (swrContext)
	{
		swr_free(&swrContext);
		swrContext = 0;
	}
	if (codecContext)
	{
		avcodec_close(codecContext);
		avcodec_free_context(&codecContext);
		codecContext = 0;
	}
	av_frame_free(&audioFrame);
	return 1;
}
AVChannelLayout AudioDecoder::MAToAV_ChannelLayout()
{
	switch (mediaStream.window.audioEngine.outputChannels)
	{
	case 1:
		return AV_CHANNEL_LAYOUT_MONO;
	case 2:
		return AV_CHANNEL_LAYOUT_STEREO;
	case 3:
		return AV_CHANNEL_LAYOUT_2_1;
	case 4:
		return AV_CHANNEL_LAYOUT_2_2;
	case 5:
		return AV_CHANNEL_LAYOUT_5POINT0;
	default:
		throw std::runtime_error("Unsupported ChannelLayout");
	}
}
AVSampleFormat AudioDecoder::MAToAV_SampleFormat()
{
	switch (mediaStream.window.audioEngine.format)
	{
	case ma_format_f32:
		return AV_SAMPLE_FMT_FLT;
	case ma_format_s32:
		return AV_SAMPLE_FMT_S32;
	case ma_format_s16:
		return AV_SAMPLE_FMT_S16;
	default:
		throw std::runtime_error("Unsupported SampleFormat");
	}
}