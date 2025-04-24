#include <zg/math/Derivative.hpp>
#include <iostream>
using Real = long double;
int main()
{
    auto f = [](const Real& x) -> Real
    {
        return x * x;
    };
    Real x = 2.5;
    auto fwd_diff = ForwardDifference<Real>(f, x);
    auto bwd_diff = BackwardDifference<Real>(f, x);
    std::cout << "fwd_diff: " << fwd_diff << std::endl  <<
                 "bwd_diff: " << bwd_diff << std::endl;
}