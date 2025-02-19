#pragma once
#include <vector>
#include <string>
#include <functional>
using namespace std;
namespace zg::crypto
{
	size_t combineHashes(size_t hash1, size_t hash2);
	template<typename T>
	size_t hashVector(const vector<T>& vec);
}