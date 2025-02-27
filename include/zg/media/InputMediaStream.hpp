#pragma once
#include <zg/Standard.hpp>
#include "I1xCoder.hpp"
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
        std::vector<std::tuple<int32_t, AVStream*, std::shared_ptr<I1xCoder>, std::queue<AVFrame *>, std::mutex>> coderStreams;
        bool playing = false;
        InputMediaStream(const std::string &uri);
        InputMediaStream(const std::string &uri, const std::shared_ptr<interfaces::IFile>& filePointer);
        size_t open();
        size_t close();
    };
}