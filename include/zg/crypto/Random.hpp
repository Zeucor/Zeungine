#pragma once
#include <random>
using namespace std;
namespace zg::crypto
{
	struct Random
	{
	private:
		inline thread_local static random_device _randomDevice = {};
		inline thread_local static mt19937 _mt19937 = mt19937(_randomDevice());

	public:
		template <typename T>
		static const T value(const T min, const T max, const size_t seed = (numeric_limits<size_t>::max)())
		{
			mt19937* mt19937Pointer = 0;
			if (seed != (numeric_limits<size_t>::max)())
			{
				mt19937Pointer = new mt19937(seed);
			}
			else
			{
				mt19937Pointer = &Random::_mt19937;
			}
			if constexpr (is_floating_point<T>::value)
			{
				uniform_real_distribution<T> distrib(min, max);
				auto value = distrib(*mt19937Pointer);
				if (seed != (numeric_limits<size_t>::max)())
				{
					delete mt19937Pointer;
					mt19937Pointer = 0;
				}
				return value;
			}
			else if constexpr (is_integral<T>::value)
			{
				uniform_int_distribution<T> distrib(min, max);
				auto value = distrib(*mt19937Pointer);
				if (seed != (numeric_limits<size_t>::max)()) // ∞
				{
					delete mt19937Pointer;
					mt19937Pointer = 0;
				}
				return value;
			}
			throw runtime_error("Type is not supported by Random::value");
		};
		template <typename T>
		static const T value(const T min, const T max, mt19937& mt19937)
		{
			if constexpr (is_floating_point<T>::value)
			{
				uniform_real_distribution<T> distrib(min, max);
				auto value = distrib(mt19937);
				return value;
			}
			else if constexpr (is_integral<T>::value)
			{
				uniform_int_distribution<T> distrib(min, max);
				auto value = distrib(mt19937);
				return value;
			}
			throw runtime_error("Type is not supported by Random::value");
		};
		template <typename T>
		static const T valueFromRandomRange(const vector<pair<T, T>>& ranges,
																				const size_t seed = (numeric_limits<size_t>::max)())
		{
			auto rangesSize = ranges.size();
			auto rangesData = ranges.data();
			size_t rangeIndex = Random::value<size_t>(0, rangesSize - 1, seed);
			auto& range = rangesData[rangeIndex];
			return Random::value(range.first, range.second, seed);
		};
		template <typename T>
		static const T valueFromRandomRange(const vector<pair<T, T>>& ranges, mt19937& mt19937)
		{
			auto rangesSize = ranges.size();
			auto rangesData = ranges.data();
			size_t rangeIndex = Random::value<size_t>(0, rangesSize - 1, mt19937);
			auto& range = rangesData[rangeIndex];
			return Random::value(range.first, range.second, mt19937);
		};
	};
} // namespace zg::crypto
