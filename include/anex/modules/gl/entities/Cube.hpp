#pragma once
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include <anex/glm.hpp>
#include <array>
namespace anex::modules::gl::entities
{
	struct Cube : GLEntity
	{
		std::vector<glm::vec4> colors;
		std::vector<glm::vec3> normals = {};
    GLScene &scene;
		inline static size_t cubesCount = 0;
		Cube(GLWindow &window, GLScene &scene, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const glm::vec3 &size, const shaders::RuntimeConstants &constants = {}, std::string_view name = "");
		void preRender() override;
	};
}