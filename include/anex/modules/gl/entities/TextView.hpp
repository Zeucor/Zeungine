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
		glm::vec2 textSize;
		glm::vec2 actualSizeBeforeNDC;
		glm::vec2 actualSize;
    fonts::freetype::FreetypeFont &font;
		float fontSize;
		bool textSizeIsNDC;
		using RepositionHandler = std::function<glm::vec3(glm::vec2 )>;
		RepositionHandler repositionHandler;
		using ResizeHandler = std::function<glm::vec2(glm::vec2 )>;
		ResizeHandler resizeHandler;
		using ReFontSizeHandler = std::function<float()>;
		ReFontSizeHandler reFontSizeHandler;
		int64_t cursorIndex = 0;
		glm::vec3 cursorPosition = glm::vec3(0);
		inline static size_t textViewsCount = 0;
		explicit TextView(GLWindow &window,
										  GLScene &scene,
										  glm::vec3 position,
										  glm::vec3 rotation,
										  glm::vec3 scale,
										  const std::string_view text,
										  glm::vec2 size,
										  fonts::freetype::FreetypeFont &font,
										  float fontSize,
										  bool textSizeIsNDC = true,
										  const RepositionHandler &repositionHandler = {},
										  const ResizeHandler &resizeHandler = {},
										  const ReFontSizeHandler &reFontSizeHandler = {},
										  std::string_view name = "");
		void update() override;
		void forceUpdate();
		void preRender() override;
		void setSize(glm::vec2 size);
		void updateText(const std::string_view text);
  };
}