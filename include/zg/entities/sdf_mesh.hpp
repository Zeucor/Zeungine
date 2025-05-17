#pragma once
#include <zg/Entity.hpp>
namespace zg::entities
{
    EntityCreateInfo sdf_mesh_factory(const std::string& sdf_key, const std::string& name, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec4 color, const shaders::RuntimeConstants& constants = {});
}