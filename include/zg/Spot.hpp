#pragma
#include <zg/glm.hpp>
namespace zg
{
    struct Spot
    {
        glm::vec3 spot;
        Spot(glm::vec3 initial_spot = {1, 1, 1});
        Spot(const Spot& other);
        Spot& operator = (const Spot& other);
        glm::vec3 operator()(float scale);
    };
}