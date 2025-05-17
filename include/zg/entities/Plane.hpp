#pragma once
#include <array>
#include <zg/Entity.hpp>
#include <zg/glm.hpp>
namespace zg::entities
{
	EntityCreateInfo PlaneFactory(glm::vec4 color = {1, 1, 1, 1}, std::string name = "", glm::vec3 position = {0, 0, 0}, glm::quat rotation = {1, 0, 0, 0}, glm::vec3 scale = {1, 1, 1},
		const shaders::RuntimeConstants constants = {}, zg::FRONTFACE frontFace = zg::IRenderer::DEFAULTFRONTFACE);
	EntityCreateInfo PlaneFactory(const std::shared_ptr<textures::Texture>& texture, std::string name = "", glm::vec3 position = {0, 0, 0}, glm::quat rotation = {1, 0, 0, 0}, glm::vec3 scale = {1, 1, 1},
		const shaders::RuntimeConstants constants = {}, zg::FRONTFACE frontFace = zg::IRenderer::DEFAULTFRONTFACE);
}
