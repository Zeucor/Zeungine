#pragma once
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include <anex/glm.hpp>
#include <array>
namespace anex::modules::gl::entities
{
	struct Plane : GLEntity
	{
		uint32_t indices[6]; // 6 faces * 2 triangles * 3 vertices
		std::array<glm::vec4, 4> colors;
		std::array<glm::vec2, 4> uvs;
		std::array<glm::vec3, 4> positions;
		std::array<glm::vec3, 4> normals = {};
    GLScene &scene;
		textures::Texture *texturePointer = 0;
		Plane(GLWindow &window, GLScene &scene, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const glm::vec2 &size, const glm::vec4 &color, const shaders::RuntimeConstants &constants = {});
		Plane(GLWindow &window, GLScene &scene, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const glm::vec2 &size, textures::Texture &texture, const shaders::RuntimeConstants &constants = {});
		void preRender() override;
	};
}