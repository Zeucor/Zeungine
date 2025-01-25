#pragma once
#include <zg/modules/gl/GLEntity.hpp>
#include <zg/modules/gl/GLScene.hpp>
#include <zg/modules/gl/GLWindow.hpp>
#include <zg/glm.hpp>
#include <array>
namespace zg::modules::gl::entities
{
	struct Cube : GLEntity
	{
		std::vector<glm::vec4> colors;
		std::vector<glm::vec3> normals = {};
    GLScene &scene;
		inline static size_t cubesCount = 0;
		Cube(GLWindow &window, GLScene &scene, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec3 size, const shaders::RuntimeConstants &constants = {}, std::string_view name = "");
		void preRender() override;
	};
}