#pragma once
#include <zg/glm.hpp>
namespace zg::math
{
    struct Rotations
    {
        static glm::vec3 Vec3AroundVec3(glm::vec3 pointToRotate, glm::vec3 rotationCenter, glm::vec3 angleDegrees);
    };
}