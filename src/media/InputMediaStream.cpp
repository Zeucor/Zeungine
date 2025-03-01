#include <zg/media/AudioDecoder.hpp>
#include <zg/media/InputMediaStream.hpp>
#include <zg/media/VideoDecoder.hpp>
using namespace zg::media;
InputMediaStream::InputMediaStream(Window& _window, const std::string& uri) : MediaStream(_window), uri(uri) { open(); }
InputMediaStream::InputMediaStream(Window& _window, const std::string& uri,
																	 const std::shared_ptr<zg::interfaces::IFile>& filePointer) :
		MediaStream(_window), uri(uri), filePointer(filePointer)
{
	open();
}
InputMediaStream::~InputMediaStream() { close(); }
size_t InputMediaStream::open()
{
	if (filePointer)
	{
		auto& file = *filePointer;
		if (!file.isOpen())
		{
			return 0;
		}
		auto streamSize = file.size();
		streamBytes = (uint8_t*)av_malloc(streamSize + 1);
		file.readBytes(0, streamSize, streamBytes);
		ioContext = avio_alloc_context(streamBytes, streamSize, 0, 0, 0, 0, 0);
		streamBytes = 0;
		if (!ioContext)
		{
			return 0;
		}
		formatContext = avformat_alloc_context();
		formatContext->pb = ioContext;
		formatContext->flags |= AVFMT_FLAG_CUSTOM_IO;
		if (avformat_open_input(&formatContext, "", 0, 0) < 0)
		{
			return 0;
		}
	}
	else
	{
		formatContext = avformat_alloc_context();
		if (avformat_open_input(&formatContext, uri.c_str(), 0, 0))
		{
			std::cerr << "Error opening '" << uri << ' formatContext' << std::endl;
			return 0;
		}
	}
	if (avformat_find_stream_info(formatContext, 0) < 0)
	{
		return 0;
	}
	// 0 audio / 1 video
	for (auto i = 0; i <= 5; ++i)
	{
		auto StreamIndex = findStreamIndex(i);
		if (StreamIndex < 0)
		{
			continue;
		}
		auto avStream = findAVStream(i, StreamIndex);
		CoderStreamTuple tuple;
		std::get<CODER_STREAM_I_STREAM_INDEX>(tuple) = StreamIndex;
		std::get<CODER_STREAM_I_STREAM>(tuple) = avStream;
		std::get<CODER_STREAM_I_FRAMEQUEUE>(tuple) = std::make_shared<zg::td::queue<AVFrame*>>();
		std::get<CODER_STREAM_I_MUTEX>(tuple) = std::make_shared<std::mutex>();
		auto i1xCoder = construct1xCoder(i, StreamIndex, avStream, std::get<CODER_STREAM_I_FRAMEQUEUE>(tuple),
																		 std::get<CODER_STREAM_I_MUTEX>(tuple));
		std::get<CODER_STREAM_I_1XCODER>(tuple) = i1xCoder;
		i1xCoder->open();
		coderStreams[i] = tuple;
		auto& tupleRef = coderStreams[coderStreams.size() - 1];
		codecIndexToStreamIndex[i] = StreamIndex;
	}
	demuxing = true;
	demuxThread = std::make_shared<std::thread>(&InputMediaStream::demuxer, this);
	return /*s*/ 1;
}
size_t InputMediaStream::close()
{
	demuxing = false;
	demuxThread->join();
	auto coderStreamsSize = coderStreams.size();
	for (int i = 0; i < coderStreamsSize; ++i)
	{
		auto& tuple = coderStreams[i];
		std::get<CODER_STREAM_I_1XCODER>(tuple)->flush();
	}
	for (int i = 0; i < coderStreamsSize; ++i)
	{
		auto& tuple = coderStreams[i];
		std::get<CODER_STREAM_I_1XCODER>(tuple)->close();
	}
	avformat_close_input(&formatContext);
	return 0;
}
void InputMediaStream::fillAudioFrames(float* frames, const int32_t& channelCount, const unsigned long& frameCount)
{
	auto& audioTuple = coderStreams[CODEC_I_AUDIO];
	auto audio1xCoder = std::dynamic_pointer_cast<AudioDecoder>(std::get<2>(audioTuple));
	auto audioDecoder = audio1xCoder.get();
	if (!audioDecoder)
		return;
    audioDecoder->fillFrames(frames, channelCount, frameCount);
}
void InputMediaStream::fillTexture(textures::Texture& texture)
{
	auto& videoTuple = coderStreams[CODEC_I_VIDEO];
	auto video1xCoder = std::dynamic_pointer_cast<VideoDecoder>(std::get<2>(videoTuple));
	auto videoDecoder = video1xCoder.get();
	if (!videoDecoder)
		return;
    videoDecoder->fillTexture(texture);
}
int32_t InputMediaStream::findStreamIndex(int i)
{
	AVMediaType mediaType;
	switch (i)
	{
	case CODEC_I_VIDEO:
		mediaType = AVMEDIA_TYPE_VIDEO;
		break;
	case CODEC_I_AUDIO:
		mediaType = AVMEDIA_TYPE_AUDIO;
		break;
	case CODEC_I_DATA:
		mediaType = AVMEDIA_TYPE_DATA;
		break;
	case CODEC_I_SUBTITLE:
		mediaType = AVMEDIA_TYPE_SUBTITLE;
		break;
	case CODEC_I_ATTACHMENT:
		mediaType = AVMEDIA_TYPE_ATTACHMENT;
		break;
	case CODEC_I_NB:
		mediaType = AVMEDIA_TYPE_NB;
		break;
	}
	return av_find_best_stream(formatContext, mediaType, -1, -1, nullptr, 0);
}
AVStream* InputMediaStream::findAVStream(int i, int32_t streamIndex)
{
	if (streamIndex < 0)
	{
		return 0;
	}
	return formatContext->streams[streamIndex];
}
std::shared_ptr<I1xCoder> InputMediaStream::construct1xCoder(int i, int32_t streamIndex, AVStream* stream, const std::shared_ptr<zg::td::queue<AVFrame*>>& frameQueuePointer, const std::shared_ptr<std::mutex>& mutexPointer)
{
	const AVCodec* codec = 0;
	AVCodecParameters* codecParameters = 0;
	codecParameters = formatContext->streams[streamIndex]->codecpar;
	codec = avcodec_find_decoder(codecParameters->codec_id);
	std::shared_ptr<I1xCoder> i1xCoder;
	switch (i)
	{
	case CODEC_I_VIDEO:
		i1xCoder = std::make_shared<VideoDecoder>(*this, codec, codecParameters, stream, frameQueuePointer, mutexPointer);
		break;
	case CODEC_I_AUDIO:
		i1xCoder = std::make_shared<AudioDecoder>(*this, codec, codecParameters, stream, frameQueuePointer, mutexPointer);
		break;
	case CODEC_I_DATA:
		// i1xCoder = std::make_shared<DataDecoder>();
		break;
	case CODEC_I_SUBTITLE:
		// i1xCoder = std::make_shared<SubtitleDecoder>();
		break;
	case CODEC_I_ATTACHMENT:
		// i1xCoder = std::make_shared<AttachmentDecoder>();
		break;
	case CODEC_I_NB:
		// i1xCoder = std::make_shared<NBDecoder>();
		break;
	}
	return i1xCoder;
}
void InputMediaStream::demuxer()
{
	auto& videoStreamIndex = codecIndexToStreamIndex[CODEC_I_VIDEO];
	auto& audioStreamIndex = codecIndexToStreamIndex[CODEC_I_AUDIO];
	int32_t ret = 0;
	bool eof = false;
	auto& audioTuple = coderStreams[CODEC_I_AUDIO];
	auto& videoTuple = coderStreams[CODEC_I_VIDEO];
	auto audio1xCoder = std::dynamic_pointer_cast<AudioDecoder>(std::get<2>(audioTuple));
	auto video1xCoder = std::dynamic_pointer_cast<VideoDecoder>(std::get<2>(videoTuple));
	auto audioDecoder = audio1xCoder.get();
	auto videoDecoder = video1xCoder.get();
	AVChannelLayout AV_CH;
	AVSampleFormat AV_SAMPLE_FMT;
	auto audioMutex = std::get<4>(audioTuple).get();
	auto videoMutex = std::get<4>(videoTuple).get();
	if (audioDecoder)
	{
		AV_CH = audioDecoder->MAToAV_ChannelLayout();
		AV_SAMPLE_FMT = audioDecoder->MAToAV_SampleFormat();
	}
	auto audioFrameQueue = std::get<3>(audioTuple).get();
	auto videoFrameQueue = std::get<3>(videoTuple).get();
	while (demuxing && !eof)
	{
		AVPacket* packet = av_packet_alloc();
		if (!packet)
			continue;
		ret = av_read_frame(formatContext, packet);
		if (ret < 0)
		{
			if ((ret == AVERROR_EOF || avio_feof(formatContext->pb)))
			{
				av_packet_unref(packet);
				av_packet_free(&packet);
				eof = true;
				continue;
			}
			else
			{
				std::cerr << "Unknown formatContext packet error" << std::endl;
				break;
			}
		}
		if (packet->stream_index == -1)
		{
			av_packet_unref(packet);
			av_packet_free(&packet);
			continue;
		}
		if (packet->stream_index == audioStreamIndex)
		{
			ret = avcodec_send_packet(audioDecoder->codecContext, packet);
			while (ret >= 0)
			{
				ret = avcodec_receive_frame(audioDecoder->codecContext, audioDecoder->audioFrame);
				if (ret == AVERROR(EAGAIN))
					break;
				else if (ret == AVERROR_EOF)
					break;
				else if (ret < 0)
				{
					std::cerr << "decoding errors" << std::endl;
					break;
				}
				auto audioSwrFrame = av_frame_alloc();
				audioSwrFrame->format = AV_SAMPLE_FMT;
				audioSwrFrame->ch_layout = AV_CH;
				audioSwrFrame->sample_rate = window.audioEngine.sampleRate;
				audioSwrFrame->nb_samples = window.audioEngine.frameSize;
				swr_convert_frame(audioDecoder->swrContext, audioSwrFrame, audioDecoder->audioFrame);
				audioSwrFrame->pts = audioDecoder->audioFrame->pts;
				audioMutex->lock();
				audioFrameQueue->push(audioSwrFrame);
				audioMutex->unlock();
			}
		}
		else if (packet->stream_index == videoStreamIndex)
		{
			ret = avcodec_send_packet(videoDecoder->codecContext, packet);
			AVFrame* videoFrame = av_frame_alloc();
			while (ret >= 0)
			{
				ret = avcodec_receive_frame(videoDecoder->codecContext, videoFrame);
				if (ret == AVERROR(EAGAIN))
					break;
				else if (ret == AVERROR_EOF)
					break;
				else if (ret < 0)
				{
					std::cerr << "decoding errors" << std::endl;
					break;
				}
				videoMutex->lock();
				videoFrameQueue->push(videoFrame);
				videoMutex->unlock();
			}
		}
		av_packet_unref(packet);
		av_packet_free(&packet);
	}
}
