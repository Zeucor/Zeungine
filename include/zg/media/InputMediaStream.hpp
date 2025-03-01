#pragma once
#include <zg/Standard.hpp>
#include "I1xCoder.hpp"
#include "MediaStream.hpp"
#include <thread>
namespace zg::media
{
	struct InputMediaStream : MediaStream
	{
		using CoderStreamTuple =
			std::tuple<int32_t, AVStream*, std::shared_ptr<I1xCoder>, zg::td::queue<AVFrame*>, std::shared_ptr<std::mutex>>;
		std::string uri;
		std::shared_ptr<interfaces::IFile> filePointer;
		unsigned char* streamBytes = 0;
		std::map<int32_t, CoderStreamTuple> coderStreams;
		bool playing = false;
		bool demuxing = false;
		std::shared_ptr<std::thread> demuxThread;
		std::unordered_map<int32_t, int32_t> codecIndexToStreamIndex;
		InputMediaStream(Window& _window, const std::string& uri);
		InputMediaStream(Window& _window, const std::string& uri, const std::shared_ptr<interfaces::IFile>& filePointer);
		~InputMediaStream();
		size_t open();
		size_t close();

	private:
		int32_t findStreamIndex(int i);
		AVStream* findAVStream(int i, int32_t streamIndex);
		std::shared_ptr<I1xCoder> construct1xCoder(int i, int32_t streamIndex, AVStream* stream);
		void demuxer();
	};
} // namespace zg::media
