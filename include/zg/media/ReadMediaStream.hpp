#pragma once
#include <thread>
#include <zg/Standard.hpp>
#include <zg/textures/Texture.hpp>
#include "MediaStream.hpp"
namespace zg::media
{
	struct ReadMediaStream : MediaStream
	{
		using CoderStreamTuple = std::tuple<int32_t, AVStream*, std::shared_ptr<I1xCoder>,
																				std::shared_ptr<zg::td::queue<AVFrame*>>, std::shared_ptr<std::mutex>>;
		std::string uri;
		std::shared_ptr<interfaces::IFile> filePointer;
		bool playing = false;
		ReadMediaStream(Window& _window, const std::string& uri);
		ReadMediaStream(Window& _window, const std::string& uri, const std::shared_ptr<interfaces::IFile>& filePointer);
		~ReadMediaStream();
		void startAudio();
		void fillNextAudioFrames(float* frames, const int32_t& channelCount, const unsigned long& frameCount);
		void fillNextVideoFrame(std::shared_ptr<textures::Texture>& texture);
		NANOSECONDS_DURATION getVideoFrameDuration() override;

	private:
		size_t open();
		size_t close();
		uint8_t* streamBytes = 0;
		std::map<int32_t, CoderStreamTuple> coderStreams;
		bool demuxing = false;
		std::shared_ptr<std::thread> demuxThread;
		std::unordered_map<int32_t, int32_t> codecIndexToStreamIndex;
		int32_t findStreamIndex(int i);
		AVStream* findAVStream(int i, int32_t streamIndex);
		std::shared_ptr<I1xCoder> construct1xCoder(int i, int32_t streamIndex, AVStream* stream,
																							 const std::shared_ptr<zg::td::queue<AVFrame*>>& frameQueuePointer,
																							 const std::shared_ptr<std::mutex>& mutexPointer);
		void demuxer();
	};
} // namespace zg::media
