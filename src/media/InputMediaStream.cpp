#include <zg/media/InputMediaStream.hpp>
using namespace zg::media;
InputMediaStream::InputMediaStream(const std::string& uri):
    uri(uri)
{
    open();
}
InputMediaStream::InputMediaStream(const std::string &uri, const std::shared_ptr<zg::interfaces::IFile>& filePointer):
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
    for (auto i = 0; i <= 1; ++i)
    {
        auto StreamIndex = findStreamIndex(i);
        auto avStream = findAVStream(i, StreamIndex);
        auto i1xCoder = construct1xCoder(i, StreamIndex, avStream);
        i1xCoder->open();
        CoderStreamTuple tuple;
        std::get<0>(tuple) = StreamIndex;
        std::get<1>(tuple) = avStream;
        std::get<2>(tuple) = i1xCoder;
        coderStreams.push_back(tuple);
    }
    return /*s*/ 1;
}
size_t InputMediaStream::close()
{
    return 0;
}
int32_t InputMediaStream::findStreamIndex(int i)
{
    return 0;
}
AVStream* InputMediaStream::findAVStream(int i, int32_t streamIndex)
{
    return 0;
}
std::shared_ptr<I1xCoder> InputMediaStream::construct1xCoder(int i, int32_t streamIndex, AVStream* stream)
{
    return 0;
}