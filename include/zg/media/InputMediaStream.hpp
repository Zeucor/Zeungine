#pragma once
#include <zg/Standard.hpp>
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
namespace zg::media
{
    struct InputMediaStream
    {
        std::string uri;
        std::shared_ptr<interfaces::IFile> filePointer;
        AVIOContext* ioContext = 0;
        AVFormatContext* formatContext = 0;
        int32_t videoStreamIndex = -1;
        AVStream* videoStream = 0;
        int audioStreamIndex = 0;
        AVStream* audioStream = 0;
        std::deque<AVFrame *> audioFrames;
        std::mutex audioFramesMutex;
        std::deque<AVFrame *> videoFrames;
        std::mutex videoFramesMutex;
        bool playing = false;
        InputMediaStream(const std::string &uri);
        InputMediaStream(const std::string &uri, const std::shared_ptr<interfaces::IFile>& filePointer);
        size_t open();
    };
}