#include <zg/system/Budget.hpp>
// QUEUE_PAIR Serial IO
Serial& serialize(Serial& serial, const QUEUE_PAIR &value)
{
	return serial << value.first << value.second;
}
Serial& deserialize(Serial& serial, QUEUE_PAIR& value)
{
	return serial >> value.first >> value.second;
}
// QUEUE Serial IO
Serial& serialize(Serial& serial, const QUEUE& que)
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
Serial& deserialize(Serial& serial, QUEUE& que)
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
