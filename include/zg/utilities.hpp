#pragma once
#include <vector>
namespace zg
{
	template <typename T>
	std::vector<T> mergeVectors(const std::vector<T> &vec1, const std::vector<T> &vec2)
	{
		std::vector<T> result;
		result.reserve(vec1.size() + vec2.size());
		result.insert(result.end(), vec1.begin(), vec1.end());
		result.insert(result.end(), vec2.begin(), vec2.end());
		return result;
	};
}