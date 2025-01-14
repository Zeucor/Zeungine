#pragma once
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
#include <array>
namespace anex::modules::gl::entities
{
	struct Toolbar : GLEntity
	{
		uint32_t indices[6];
		std::array<glm::vec4, 4> colors;
		std::array<glm::vec3, 4> positions;
    GLScene &scene;
		fonts::freetype::FreetypeFont &font;
  	Toolbar(GLWindow &window,
						GLScene &scene,
						const glm::vec3 &position,
						const glm::vec3 &rotation,
						const glm::vec3 &scale,
						const glm::vec4 &color,
						const float &height,
						fonts::freetype::FreetypeFont &font);
    void preRender() override;
  };
}