#include <zg/media/InputMediaStream.hpp>
#include <zg/media/VideoDecoder.hpp>
#include <zg/media/AudioDecoder.hpp>
using namespace zg::media;
InputMediaStream::InputMediaStream(Window& _window, const std::string& uri):
    MediaStream(_window),
    uri(uri)
{
    open();
}
InputMediaStream::InputMediaStream(Window& _window, const std::string &uri, const std::shared_ptr<zg::interfaces::IFile>& filePointer):
    MediaStream(_window),
    uri(uri),
    filePointer(filePointer)
{
    open();
}
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
        auto i1xCoder = construct1xCoder(i, StreamIndex, avStream);
        i1xCoder->open();
        CoderStreamTuple tuple;
        std::get<CODER_STREAM_I_STREAM_INDEX>(tuple) = StreamIndex;
        std::get<CODER_STREAM_I_STREAM>(tuple) = avStream;
        std::get<CODER_STREAM_I_1XCODER>(tuple) = i1xCoder;
        coderStreams.emplace_back(tuple);
        auto &tupleRef = coderStreams[coderStreams.size() - 1];
        std::get<CODER_STREAM_I_MUTEX>(tupleRef) = std::make_shared<std::mutex>();
    }
    return /*s*/ 1;
}
size_t InputMediaStream::close()
{
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
std::shared_ptr<I1xCoder> InputMediaStream::construct1xCoder(int i, int32_t streamIndex, AVStream* stream)
{
    const AVCodec* codec = 0;
    AVCodecParameters* codecParameters = 0;
    codecParameters = formatContext->streams[streamIndex]->codecpar;
    codec = avcodec_find_decoder(codecParameters->codec_id);
    std::shared_ptr<I1xCoder> i1xCoder;
    switch (i)
    {
        case CODEC_I_VIDEO:
            i1xCoder = std::make_shared<VideoDecoder>(*this, codec, codecParameters, stream);
            break;
        case CODEC_I_AUDIO:
            i1xCoder = std::make_shared<AudioDecoder>(*this, codec, codecParameters, stream);
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