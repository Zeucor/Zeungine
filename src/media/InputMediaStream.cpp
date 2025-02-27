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
    return /*s*/ 1;
}
size_t InputMediaStream::close()
{
    return 0;
}