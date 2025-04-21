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
    size_t* _6Index = 0, * _8Index = 0;
    for (size_t c = 1; c <= 10; c++)
    {
        std::string str;
        for (size_t cc = 1; cc <= 5; ++cc)
        {
            str += (char)crypto::Random::value<uint16_t>(65, 92);
        }
        auto emplace_tuple = floatingVector.emplace_back_key(crypto::Random::value<float>(1, 100), str);
        if (c == 6)
            _6Index = std::get<KEY_ID_VECTOR_INDEX_INDEX>(emplace_tuple);
        else if (c == 8)
            _8Index = std::get<KEY_ID_VECTOR_INDEX_INDEX>(emplace_tuple);
    }
    auto keyIter = floatingVector.key_begin();
    auto keyEnd = floatingVector.key_end();
    std::cout << "Before Remove 5" << std::endl;
    auto _6ValueIter = floatingVector.begin() + *_6Index;
    std::cout << "_6IndexPointer: " << _6Index << ", _6Index: " << *_6Index << ", _6Val: " << *_6ValueIter << std::endl;
    auto _8ValueIter = floatingVector.begin() + *_8Index;
    std::cout << "_8IndexPointer: " << _8Index << ", _8Index: " << *_8Index << ", _8Val: " << *_8ValueIter << std::endl;
    while (keyIter != keyEnd)
    {
        std::cout << "Key: " << keyIter.key() << ", Value: " << *keyIter << std::endl;
        ++keyIter;
    }
    for (auto c = 1; c <= 5; c++)
        floatingVector.erase(floatingVector.id_begin());
    std::cout << "After Remove 5" << std::endl;
    _6ValueIter = floatingVector.begin() + *_6Index;
    std::cout << "_6IndexPointer: " << _6Index << ", _6Index: " << *_6Index << ", _6Val: " << *_6ValueIter << std::endl;
    _8ValueIter = floatingVector.begin() + *_8Index;
    std::cout << "_8IndexPointer: " << _8Index << ", _8Index: " << *_8Index << ", _8Val: " << *_8ValueIter << std::endl;
    return 0;
}