#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/glm.hpp>
#include <array>
namespace zg::entities
{
	struct Cube : Entity
	{
		std::vector<glm::vec4> colors;
		std::vector<glm::vec3> normals = {};
		Scene &scene;
		inline static size_t cubesCount = 0;
		Cube(Window &window,
			 Scene &scene,
			 glm::vec3 position,
			 glm::vec3 rotation,
			 glm::vec3 scale,
			 glm::vec3 size,
			 const shaders::RuntimeConstants &constants = {},
			 std::string_view name = "");
		bool preRender() override;
	};
}