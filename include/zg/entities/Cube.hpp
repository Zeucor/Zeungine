#pragma once
#include <zg/Entity.hpp>
#include <zg/interfaces/IRenderer.hpp>
namespace zg::entities
{
	EntityCreateInfo CubeFactory(std::string name = "", glm::vec3 position = {0, 0, 0}, glm::quat rotation = {1, 0, 0, 0}, glm::vec3 scale = {1, 1, 1}, glm::vec3 size = {1, 1, 1}, glm::vec4 color = {1,0,0,1}, const shaders::RuntimeConstants& constants = {}, zg::FRONTFACE frontFace = zg::IRenderer::DEFAULTFRONTFACE);
} // namespace zg::entities
