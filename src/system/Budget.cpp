#include <zg/system/Budget.hpp>
using namespace std;
namespace zgfilesystem
{
	// QUEUE_PAIR Serial IO
	template<typename WriteStreamT, typename ReadStreamT>
	Serial<WriteStreamT, ReadStreamT>& serialize(Serial<WriteStreamT, ReadStreamT>& serial, const QUEUE_PAIR &value)
	{
		return serial << value.first << value.second;
	}
	template Serial<std::fstream, std::fstream>& zgfilesystem::serialize(Serial<std::fstream, std::fstream>&, const QUEUE_PAIR &);
	template <typename WriteStreamT, typename ReadStreamT>
	Serial<WriteStreamT, ReadStreamT>& deserialize(Serial<WriteStreamT, ReadStreamT>& serial, QUEUE_PAIR& value)
	{
		return serial >> value.first >> value.second;
	}
	template Serial<std::fstream, std::fstream>& zgfilesystem::deserialize(Serial<std::fstream, std::fstream>&, QUEUE_PAIR &);
	// QUEUE Serial IO
	template <typename WriteStreamT, typename ReadStreamT>
	Serial<WriteStreamT, ReadStreamT>& serialize(Serial<WriteStreamT, ReadStreamT>& serial, const QUEUE& que)
	{
		serial << que.size();
		QUEUE st = que;
		while (!st.empty())
		{
			auto marker = st.front();
			serial << marker;
			st.pop();
		}
		return serial;
	}
	template Serial<std::fstream, std::fstream>& zgfilesystem::serialize(Serial<std::fstream, std::fstream>&, const QUEUE &);
	template <typename WriteStreamT, typename ReadStreamT>
	Serial<WriteStreamT, ReadStreamT>& deserialize(Serial<WriteStreamT, ReadStreamT>& serial, QUEUE& que)
	{
		auto size_ = que.size();
		serial >> size_;
		for (int count = 1; count <= size_; ++count)
		{
			QUEUE_PAIR value;
			serial >> value;
			que.push(value);
		}
		return serial;
	}
	template Serial<std::fstream, std::fstream>& zgfilesystem::deserialize(Serial<std::fstream, std::fstream>&, QUEUE &);
} // namespace zgfilesystem
