#pragma once
#include <zg/ffmpeg.hpp>
#include <zg/Window.hpp>
#include <zg/system/Budget.hpp>
using namespace zg::budget;
namespace zg::media
{
#define CODER_STREAM_INDEX_STREAM_INDEX 0
#define CODER_STREAM_INDEX_STREAM 1
#define CODER_STREAM_INDEX_1XCODER 2
#define CODER_STREAM_INDEX_FRAMEQUEUE 3
#define CODER_STREAM_INDEX_MUTEX 4
#define CODEC_INDEX_VIDEO 0
#define CODEC_INDEX_AUDIO 1
#define CODEC_INDEX_DATA 2
#define CODEC_INDEX_SUBTITLE 3
#define CODEC_INDEX_ATTACHMENT 4
#define CODEC_INDEX_NB 5
	struct MediaStream
	{
		Window& window;
		AVIOContext* ioContext = 0;
		AVFormatContext* formatContext = 0;
		MediaStream(Window& _window);
		virtual NANOSECONDS_DURATION getVideoFrameDuration() = 0;
	};
} // namespace zg::media
