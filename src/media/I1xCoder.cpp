#include <zg/media/I1xCoder.hpp>
using namespace zg::media;
I1xCoder::I1xCoder(const std::shared_ptr<zg::td::queue<AVFrame*>>& frameQueuePointer,
									 const std::shared_ptr<std::mutex>& mutexPointer) :
		frameQueuePointer(frameQueuePointer), mutexPointer(mutexPointer)
{
}
