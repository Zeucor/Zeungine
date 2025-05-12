#pragma once
#include <zg/Entity.hpp>
namespace zg::entities
{
    EntityCreateInfo SphereFactory(glm::vec4 color, const std::string& name = "Sphere", glm::vec3 position = {0, 0, 0}, glm::quat rotation = {1, 0, 0, 0}, glm::vec3 scale = {1, 1, 1},
        const shaders::RuntimeConstants& constants = {});
}