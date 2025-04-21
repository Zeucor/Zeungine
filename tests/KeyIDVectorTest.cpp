#include <zg/KeyIDVector.hpp>
#include <string>
#include <iostream>
#include <chrono>
#include <zg/crypto/Random.hpp>
#include <cassert>
using namespace zg;
struct Structure
{
    float x;
    float o;
    float v;
};
int main()
{
    // auto timeStart = std::chrono::system_clock::now();
    // KeyIDVector<std::string, Structure> vector([](const auto& structure)
    // {
    //     return "{ " + std::to_string(structure.x) + ", " + std::to_string(structure.o) + ", " + std::to_string(structure.v) + " }";
    // });
    // for (size_t c = 1; c <= 5000; c++)
    // {
    //     vector.emplace_back(crypto::Random::value<float>(-421, 328000), crypto::Random::value<float>(1, 8), crypto::Random::value<float>(0, 4));
    // }
    // auto timeEnd = std::chrono::system_clock::now();
    // auto timeTaken = timeEnd - timeStart;
    // std::cout << "time taken: " << (timeTaken.count() / 1'000'000'000.0L) << "s" << std::endl << std::endl;
    // auto vectorSize = vector.size();
    // auto vectorData = vector.data();
    // for (auto index = 0; index < vectorSize; ++index)
    // {
    //     auto iter = vector.begin() + index;
    //     auto key = vector.getKey(iter);
    //     // std::cout << "key[" << index << "]: " << key << std::endl;
    //     assert(iter == vector.find_key(key));
    // }

    KeyIDVector<float, std::string> floatingVector;
    for (size_t c = 1; c <= 10; c++)
    {
        std::string str;
        for (size_t cc = 1; cc <= 5; ++cc)
        {
            str += (char)crypto::Random::value<uint16_t>(65, 92);
        }
        floatingVector.emplace_back_key(crypto::Random::value<float>(1, 100), str);
    }
    auto keyIter = floatingVector.key_begin();
    auto keyEnd = floatingVector.key_end();
    while (keyIter != keyEnd)
    {
        std::cout << "Key: " << keyIter.key() << ", Value: " << *keyIter << std::endl;
        ++keyIter;
    }
}