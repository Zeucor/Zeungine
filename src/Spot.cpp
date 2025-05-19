#include <zg/Spot.hpp>
using namespace zg;
Spot::Spot(glm::vec3 initial_spot):
    spot(initial_spot)
{}
Spot::Spot(const Spot& other):
    spot(other.spot)
{}
Spot& Spot::operator = (const Spot& other)
{
    spot = other.spot;
    return *this;
}
glm::vec3 Spot::operator()(float scale)
{
    spot.x += scale / 2.f;
    auto spot_copy_bb_et = spot;
    spot.x += scale / 2.f;
    return spot_copy_bb_et;
}