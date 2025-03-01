#pragma once
#include <zg/ffmpeg.hpp>
#include <zg/Window.hpp>
namespace zg::media
{
#define CODER_STREAM_I_STREAM_INDEX 0
#define CODER_STREAM_I_STREAM 1
#define CODER_STREAM_I_1XCODER 2
#define CODER_STREAM_I_FRAMEQUEUE 3
#define CODER_STREAM_I_MUTEX 4
#define CODEC_I_VIDEO 0
#define CODEC_I_AUDIO 1
#define CODEC_I_DATA 2
#define CODEC_I_SUBTITLE 3
#define CODEC_I_ATTACHMENT 4
#define CODEC_I_NB 5
	struct MediaStream
	{
		Window& window;
		AVIOContext* ioContext = 0;
		AVFormatContext* formatContext = 0;
		MediaStream(Window& _window);
	};
} // namespace zg::media
