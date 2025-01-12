#pragma once
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GL.hpp>
#include <anex/glm.hpp>
#include <array>
namespace anex::modules::gl::entities
{
	struct Cube : GLEntity
	{
		uint32_t indices[36]; // 6 faces * 2 triangles * 3 vertices
		std::array<glm::vec4, 24> colors;
		std::array<glm::vec3, 24> positions;
		std::array<glm::vec3, 24> normals = {};
    GLScene &scene;
		Cube(GLWindow &window, GLScene &scene, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const glm::vec3 &size, const shaders::RuntimeConstants &constants = {});
		void preRender() override;
	};
}