#pragma once
#include <anex/modules/gl/textures/Texture.hpp>
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
#include <array>
namespace anex::modules::gl::entities
{
	struct TextView : GLEntity
	{
		uint32_t indices[6];
		std::array<glm::vec3, 4> positions;
		std::array<glm::vec2, 4> uvs;
		std::array<glm::vec3, 4> normals = {};
		std::shared_ptr<textures::Texture> texturePointer;
		GLScene &scene;
	    std::string oldText;
	    std::string text;
	    fonts::freetype::FreetypeFont &font;
		float fontSize;
		explicit TextView(GLWindow &window,
						  GLScene &scene,
						  const glm::vec3 &position,
						  const glm::vec3 &rotation,
						  const glm::vec3 &scale,
						  const std::string &text,
						  const glm::vec2 &size,
						  fonts::freetype::FreetypeFont &font,
						  const float &fontSize);
		void update() override;
		void preRender() override;
  };
}