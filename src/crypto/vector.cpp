#include <anex/crypto/vector.hpp>
std::size_t anex::crypto::combineHashes(size_t hash1, size_t hash2)
{
	return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2)); // Knuth's hash combining
};
template <typename T>
std::size_t anex::crypto::hashVector(const std::vector<T>& vec)
{
	std::size_t combinedHash = 0;
	for (const auto& val : vec)
	{
		auto valHash = std::hash<T>{}(val); // Hash each value
		combinedHash = combineHashes(combinedHash, valHash); // Combine the hashes
	}
	return combinedHash;
};
template size_t anex::crypto::hashVector<std::string_view>(const std::vector<std::string_view> &);
template size_t anex::crypto::hashVector<size_t>(const std::vector<size_t> &);