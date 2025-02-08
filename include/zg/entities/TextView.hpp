#pragma once
#include <zg/textures/Texture.hpp>
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <array>
namespace zg::entities
{
	struct TextView : Entity
	{
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals = {};
		std::shared_ptr<textures::Texture> texturePointer;
		Scene &scene;
		glm::vec4 textColor;
		std::string oldText;
		std::string text;
		glm::vec2 size;
		glm::vec2 textSize;
		glm::vec2 actualSizeBeforeNDC;
		glm::vec2 actualSize;
		fonts::freetype::FreetypeFont &font;
		float fontSize;
		bool textSizeIsNDC;
		using RepositionHandler = std::function<glm::vec3(glm::vec2)>;
		RepositionHandler repositionHandler;
		using ResizeHandler = std::function<glm::vec2(glm::vec2)>;
		ResizeHandler resizeHandler;
		using ReFontSizeHandler = std::function<float()>;
		ReFontSizeHandler reFontSizeHandler;
		int64_t cursorIndex = 0;
		glm::vec3 cursorPosition = glm::vec3(0);
		IWindow::EventIdentifier resizeID = 0;
		inline static size_t textViewsCount = 0;
		explicit TextView(Window &window,
						  Scene &scene,
						  glm::vec3 position,
						  glm::vec3 rotation,
						  glm::vec3 scale,
						  glm::vec4 textColor,
						  const std::string_view text,
						  glm::vec2 size,
						  fonts::freetype::FreetypeFont &font,
						  float fontSize,
						  bool textSizeIsNDC = true,
						  const RepositionHandler &repositionHandler = {},
						  const ResizeHandler &resizeHandler = {},
						  const ReFontSizeHandler &reFontSizeHandler = {},
						  std::string_view name = "");
		~TextView() override;
		void update() override;
		void forceUpdate();
		bool preRender() override;
		void setSize(glm::vec2 size);
		void updateText(const std::string_view text);
		void setTextColor(glm::vec4 newTextColor);
		void forceReposition();
	};
}