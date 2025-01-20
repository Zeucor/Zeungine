#pragma once
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/glm.hpp>
#include <array>
namespace anex::modules::gl
{
	struct GLWindow;
}
namespace anex::modules::gl::entities
{
	struct Plane : GLEntity
	{
		std::vector<glm::vec4> colors;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals = {};
    GLScene &scene;
		textures::Texture *texturePointer = 0;
		glm::vec2 size;
		inline static size_t planesCount = 0;
		Plane(GLWindow &window,
					GLScene &scene,
					glm::vec3 position,
					glm::vec3 rotation,
					glm::vec3 scale,
					glm::vec2 size,
					glm::vec4 color,
					const shaders::RuntimeConstants &constants = {},
					std::string_view name = "");
		Plane(GLWindow &window,
					GLScene &scene,
					glm::vec3 position,
					glm::vec3 rotation,
					glm::vec3 scale,
					glm::vec2 size,
					textures::Texture &texture,
					const shaders::RuntimeConstants &constants = {},
					std::string_view name = "");
		void preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec2 size);
	};
}