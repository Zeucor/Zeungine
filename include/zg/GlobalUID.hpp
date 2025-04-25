#pragma once
#include <mutex>
namespace zg
{
	struct GlobalUID
	{
	private:
		static inline size_t Count = 0;
		static inline std::mutex Mutex = {};

	public:
		static size_t GetNew()
		{
			std::lock_guard lock(Mutex);
			return ++Count;
		}
	};
} // namespace zg
