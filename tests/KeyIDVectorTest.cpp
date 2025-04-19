#include <zg/KeyIDVector.hpp>
#include <string>
#include <iostream>
#include <chrono>
#include <zg/crypto/Random.hpp>
#include <cassert>
struct Structure
{
    float x;
    float o;
    float v;
};
int main()
{
    auto timeStart = std::chrono::system_clock::now();
    zg::KeyIDVector<std::string, Structure> vector([](const auto& structure)
    {
        return "{ " + std::to_string(structure.x) + ", " + std::to_string(structure.o) + ", " + std::to_string(structure.v) + " }";
    });
    for (size_t c = 1; c <= 5000; c++)
    {
        vector.emplace_back(zg::crypto::Random::value<float>(-421, 328000), zg::crypto::Random::value<float>(1, 8), zg::crypto::Random::value<float>(0, 4));
    }
    auto timeEnd = std::chrono::system_clock::now();
    auto timeTaken = timeEnd - timeStart;
    std::cout << "time taken: " << (timeTaken.count() / 1'000'000'000.0L) << "s" << std::endl << std::endl;
    auto vectorSize = vector.size();
    auto vectorData = vector.data();
    for (auto index = 0; index < vectorSize; ++index)
    {
        auto iter = vector.begin() + index;
        auto key = vector.getKey(iter);
        // std::cout << "key[" << index << "]: " << key << std::endl;
        assert(iter == vector.find_key(key));
    }
}