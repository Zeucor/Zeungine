#pragma once
#include <zg/Standard.hpp>
#include "I1xCoder.hpp"
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libavutil/timestamp.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}
namespace zg::media
{
	struct InputMediaStream
	{
		using CoderStreamTuple =
			std::tuple<int32_t, AVStream*, std::shared_ptr<I1xCoder>, std::queue<AVFrame*>, std::mutex>;
		std::string uri;
		std::shared_ptr<interfaces::IFile> filePointer;
		unsigned char* streamBytes = 0;
		AVIOContext* ioContext = 0;
		AVFormatContext* formatContext = 0;
		std::vector<CoderStreamTuple> coderStreams;
		bool playing = false;
		InputMediaStream(const std::string& uri);
		InputMediaStream(const std::string& uri, const std::shared_ptr<interfaces::IFile>& filePointer);
		size_t open();
		size_t close();

	private:
		int32_t findStreamIndex(int i);
		AVStream* findAVStream(int i, int32_t streamIndex);
		std::shared_ptr<I1xCoder> construct1xCoder(int i, int32_t streamIndex, AVStream* stream);
	};
} // namespace zg::media
