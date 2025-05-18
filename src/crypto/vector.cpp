#include <zg/crypto/vector.hpp>
#include <zg/glm.hpp>
std::size_t zg::crypto::combineHashes(size_t hash1, size_t hash2)
{
	return hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2)); // Knuth's hash combining
};
template <typename T>
std::size_t zg::crypto::hashVector(const std::vector<T> &vec)
{
	std::size_t combinedHash = 0;
	for (const auto &val : vec)
	{
		auto valHash = std::hash<T>{}(val);					 // Hash each value
		combinedHash = combineHashes(combinedHash, valHash); // Combine the hashes
	}
	return combinedHash;
};
template size_t zg::crypto::hashVector<std::string_view>(const std::vector<std::string_view> &);
template size_t zg::crypto::hashVector<std::string>(const std::vector<std::string> &);
template size_t zg::crypto::hashVector<size_t>(const std::vector<size_t> &);
template size_t zg::crypto::hashVector<uint32_t>(const std::vector<uint32_t> &);
template size_t zg::crypto::hashVector<glm::vec2>(const std::vector<glm::vec2> &);
template size_t zg::crypto::hashVector<glm::vec3>(const std::vector<glm::vec3> &);
template size_t zg::crypto::hashVector<glm::vec4>(const std::vector<glm::vec4> &);
