#include <zg/system/Budget.hpp>

std::string zg::ns_to_s_string(size_t ns)
{
	if (ns < 1'000)
	{
		return std::to_string(ns) + "ns";
	}
	else if (ns < 1'000'000)
	{
		return std::to_string(ns / 1'000.0L) + "us";
	}
	else if (ns < 1'000'000'000)
	{
		return std::to_string(ns / 1'000'000.0L) + "ms";
	}
	else if (ns < 1'000'000'000'000)
	{
		return std::to_string(ns / 1'000'000'000.0L) + "s";
	}
	return std::to_string(ns / 1'000'000'000'000.0L) + "ks";
}
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
