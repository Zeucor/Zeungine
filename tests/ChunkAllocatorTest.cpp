#include <zg/ChunkAllocator.hpp>
#include <vector>
#include <chrono>
#include <iostream>
#include <zg/crypto/Random.hpp>
int main()
{
    zg::ChunkAllocator<char> allocator;
    auto mem = allocator.allocate(1024);
    allocator.deallocate(mem, 1024);
    std::vector<size_t, zg::ChunkAllocator<size_t>> vec;
    auto start = std::chrono::system_clock::now();
    for (size_t c = 1; c < 10240; c++)
        vec.push_back(zg::crypto::Random::value<size_t>(1, 128));
    auto end = std::chrono::system_clock::now();
    auto diff = end - start;
    std::cout << (diff.count() / 1'000'000'000.0L) << std::endl;
    return 0;
}