#include <cassert>
#include <iostream>
#include <stdint.h>
#include <zg/queue.hpp>
int main()
{
	zg::td::queue<int32_t> qu;
	qu.push(1);
	qu.push(2);
	std::cout << "qu-front: " << qu.front() << ", qu-back: " << qu.back() << std::endl;
	qu.pop();
	qu.push(3);
	std::cout << "qu-front: " << qu.front() << ", qu-back: " << qu.back() << std::endl;
	qu.pop();
	std::cout << "qu-front: " << qu.front() << ", qu-back: " << qu.back() << std::endl;
	qu.pop();
	assert(qu.empty());
}
