#pragma once
#include <zg/Standard.hpp>
#include "I1xCoder.hpp"
#include <zg/ffmpeg.hpp>
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
	struct InputMediaStream
	{
		using CoderStreamTuple =
			std::tuple<int32_t, AVStream*, std::shared_ptr<I1xCoder>, zg::td::queue<AVFrame*>, std::shared_ptr<std::mutex>>;
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
