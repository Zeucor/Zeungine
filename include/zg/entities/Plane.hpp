#pragma once
#include <array>
#include <zg/Entity.hpp>
#include <zg/glm.hpp>
namespace zg::entities
{
	enum class PlaneType
	{
		XZ_Center = 4,
		XY_Center = 5,
		YZ_Center = 6,
		XY_BottomLeft = 7
	};
	EntityCreateInfo PlaneFactory(
		glm::vec4 color = {1, 1, 1, 1},
		std::string name = "",
		glm::vec3 position = {0, 0, 0},
		glm::quat rotation = {1, 0, 0, 0},
		glm::vec3 scale = {1, 1, 1},
		const shaders::RuntimeConstants constants = {},
		PlaneType planeType = PlaneType::XY_Center,
		zg::FRONTFACE frontFace = zg::IRenderer::DEFAULTFRONTFACE
	);
	EntityCreateInfo PlaneFactory(
		const std::shared_ptr<textures::Texture>& texture,
		std::string name = "",
		glm::vec3 position = {0, 0, 0},
		glm::quat rotation = {1, 0, 0, 0},
		glm::vec3 scale = {1, 1, 1},
    	const std::vector<glm::vec2>& uv2s = {},
		const shaders::RuntimeConstants constants = {},
		PlaneType planeType = PlaneType::XY_Center,
		zg::FRONTFACE frontFace = zg::IRenderer::DEFAULTFRONTFACE
	);
}
