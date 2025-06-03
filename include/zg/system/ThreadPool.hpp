#pragma once
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <tuple>
#include <future>
namespace zg::system
{
	struct ThreadPool
	{
        using Task = std::function<void()>;
        using Promise = std::shared_ptr<std::promise<bool>>;
        using TaskTuple = std::tuple<Task, Promise>;
	private:
		std::vector<std::thread> threads;
		std::mutex mutex;
        std::condition_variable cv;
        std::queue<TaskTuple> taskQueue;
        bool stop;
	public:
		ThreadPool():
            stop(false)
		{
            auto tc = 16;
			threads.reserve(tc);
			for (size_t c = 1; c <= tc; c++)
			{
				threads.emplace_back(&ThreadPool::threadMain, this);
			}
		}
        ~ThreadPool()
        {
            shutdown();
            for (auto& t : threads)
                if (t.joinable())
                    t.join();
        }
		void threadMain()
		{
			while (true)
			{
                TaskTuple tuple;
                {
                    std::unique_lock lock(mutex);
                    cv.wait(lock, [&](){
                        return !taskQueue.empty() || stop;
                    });
                    if (stop)
                        break;
                    tuple = taskQueue.front();
                    taskQueue.pop();
                }
                std::get<0>(tuple)();
                std::get<1>(tuple)->set_value(true);
			}
		}
        void shutdown()
        {
            {
                std::unique_lock lock(mutex);
                if (stop)
                    return;
                stop = true;
            }
            cv.notify_all();
        }
        Promise emplace_task(const Task& task)
        {
            auto promise = std::make_shared<std::promise<bool>>();
            {
                std::unique_lock lock(mutex);
                taskQueue.push({task, promise});
            }
            cv.notify_one();
            return promise;
        }
	};
} // namespace zg::system
