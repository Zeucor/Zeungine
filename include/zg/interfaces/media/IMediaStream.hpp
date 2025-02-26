#pragma once
#include <string>
#include <deque>
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
#include <libavutil/avutil.h>
}
namespace zg::interfaces::media
{
    struct IMediaStream
    {
        std::string uri;
        AVIOContext* ioContext = 0;
        AVFormatContext* formatContext = 0;
        int32_t videoStreamIndex = -1;
        AVStream* videoStream = 0;
        int audioStreamIndex = 0;
        AVStream* audioStream = 0;
        std::deque<AVFrame *> audioFrames;
        std::deque<AVFrame *> videoFrames;
        bool encoding = false;
        bool playing = false;
        IMediaStream(const std::string &uri, bool encodingVideo = false, bool encodingAudio - false(l)
    };
}