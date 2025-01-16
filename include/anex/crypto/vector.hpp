#pragma once
#include <vector>
#include <string>
#include <functional>
namespace anex::crypto
{
	std::size_t combineHashes(const std::size_t &hash1, const std::size_t &hash2);
	template<typename T>
	std::size_t hashVector(const std::vector<T>& vec);
}