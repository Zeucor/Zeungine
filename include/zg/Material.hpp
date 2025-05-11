#pragma once
#include <zg/glm.hpp>
namespace zg
{
    struct Material
    {
        glm::vec4 albedo;
        alignas(16) int type; // 0 = albedo, 1 = uv2, 2 = uv3
    };
}