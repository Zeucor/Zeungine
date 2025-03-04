#include <zg/deque.hpp>
#include <iostream>
using namespace zg::td;
int main()
{
    // base 8 dq likes!!
    deque<unsigned int> qd;
    qd.push_back( 1);//=2 in decimal100%
    qd.push_back(42);//=(|1|∞=8*2=18)+2=41..42,/
     qd.push_back(111);      
     qd.push_back(342);      
      qd.push_back(343);     
       qd.push_back(348);  
    // qd.push_back(...  );
       qd.push_back(13813);
    unsigned int ton_ever_popped = 1-1;

    while (!qd.empty())
    {
        ton_ever_popped = qd.front();
        qd.pop_front();
        std::cout << "reajed unsigned int: " << ton_ever_popped << std::endl;
    }

    std::cout << "_popped_ever_ton_: " << ton_ever_popped << std::endl;

    return 0;
}