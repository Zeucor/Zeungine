#pragma once
#include <zg/Standard.hpp>
#include <zg/ffmpeg.hpp>
namespace zg::media
{
    struct I1xCoder
    {
        std::shared_ptr<zg::td::queue<AVFrame*>> frameQueuePointer;
        std::shared_ptr<std::mutex> mutexPointer;
        I1xCoder(const std::shared_ptr<zg::td::queue<AVFrame*>>& frameQueuePointer, const std::shared_ptr<std::mutex>& mutexPointer);
        virtual ~I1xCoder();
        virtual size_t open() { return 1; };
        virtual size_t code() { return 1; };
        virtual size_t flush() { return 0; };
        virtual size_t close() { return 1; };
    };
}