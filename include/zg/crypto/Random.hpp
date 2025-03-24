#pragma once
#include <random>
namespace zg::crypto
{
	struct Random
	{
	private:
		inline thread_local static std::random_device _randomDevice = {};
		inline thread_local static std::mt19937 _mt19937 = std::mt19937(_randomDevice());

	public:
		template <typename T>
		static const T value(const T min, const T max, const size_t seed = (std::numeric_limits<size_t>::max)())
		{
			std::mt19937* mt19937Pointer = 0;
			if (seed != (std::numeric_limits<size_t>::max)())
			{
				mt19937Pointer = new std::mt19937(seed);
			}
			else
			{
				mt19937Pointer = &Random::_mt19937;
			}
			if constexpr (std::is_floating_point<T>::value)
			{
				std::uniform_real_distribution<T> distrib(min, max);
				auto value = distrib(*mt19937Pointer);
				if (seed != (std::numeric_limits<size_t>::max)())
				{
					delete mt19937Pointer;
					mt19937Pointer = 0;
				}
				return value;
			}
			else if constexpr (std::is_integral<T>::value)
			{
				std::uniform_int_distribution<T> distrib(min, max);
				auto value = distrib(*mt19937Pointer);
				if (seed != (std::numeric_limits<size_t>::max)()) // ∞
				{
					delete mt19937Pointer;
					mt19937Pointer = 0;
				}
				return value;
			}
			throw std::runtime_error("Type is not supported by Random::value");
		};
		template <typename T>
		static const T value(const T min, const T max, std::mt19937& mt19937)
		{
			if constexpr (std::is_floating_point<T>::value)
			{
				std::uniform_real_distribution<T> distrib(min, max);
				auto value = distrib(mt19937);
				return value;
			}
			else if constexpr (std::is_integral<T>::value)
			{
				std::uniform_int_distribution<T> distrib(min, max);
				auto value = distrib(mt19937);
				return value;
			}
			throw std::runtime_error("Type is not supported by Random::value");
		};
		template <typename T>
		static const T valueFromRandomRange(const std::vector<std::pair<T, T>>& ranges,
																				const size_t seed = (std::numeric_limits<size_t>::max)())
		{
			auto rangesSize = ranges.size();
			auto rangesData = ranges.data();
			size_t rangeIndex = Random::value<size_t>(0, rangesSize - 1, seed);
			auto& range = rangesData[rangeIndex];
			return Random::value(range.first, range.second, seed);
		};
		template <typename T>
		static const T valueFromRandomRange(const std::vector<std::pair<T, T>>& ranges, std::mt19937& mt19937)
		{
			auto rangesSize = ranges.size();
			auto rangesData = ranges.data();
			size_t rangeIndex = Random::value<size_t>(0, rangesSize - 1, mt19937);
			auto& range = rangesData[rangeIndex];
			return Random::value(range.first, range.second, mt19937);
		};
	};
} // namespace zg::crypto
