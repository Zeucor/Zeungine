#pragma once
#include <vector>
#include <string>
#include <functional>
namespace anex::crypto
{
	std::size_t combineHashes(const std::size_t &hash1, const std::size_t &hash2);
	std::size_t hashVectorOfStringViews(const std::vector<std::string_view>& vec);
}