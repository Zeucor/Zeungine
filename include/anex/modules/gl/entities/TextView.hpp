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
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals = {};
		std::shared_ptr<textures::Texture> texturePointer;
		GLScene &scene;
    std::string oldText;
    std::string text;
		glm::vec2 size;
    fonts::freetype::FreetypeFont &font;
		float fontSize;
		inline static size_t textViewsCount = 0;
		explicit TextView(GLWindow &window,
										  GLScene &scene,
										  const glm::vec3 &position,
										  const glm::vec3 &rotation,
										  const glm::vec3 &scale,
										  const std::string &text,
										  const glm::vec2 &size,
										  fonts::freetype::FreetypeFont &font,
										  const float &fontSize,
										  const std::string &name = "");
		void update() override;
		void preRender() override;
		void setSize(const glm::vec2 &size);
  };
}