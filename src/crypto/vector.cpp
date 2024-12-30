#include <anex/crypto/vector.hpp>
std::size_t anex::crypto::combineHashes(const std::size_t &hash1, const std::size_t &hash2)
{
	return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2)); // Knuth's hash combining
};
std::size_t anex::crypto::hashVectorOfStrings(const std::vector<std::string>& vec)
{
	std::size_t combinedHash = 0;
	for (const auto& str : vec)
	{
		std::size_t strHash = std::hash<std::string>{}(str); // Hash each string
		combinedHash = combineHashes(combinedHash, strHash); // Combine the hashes
	}
	return combinedHash;
};