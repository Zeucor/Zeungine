#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/fonts/freetype/Freetype.hpp>

namespace zg::entities
{
	struct Button : Entity
	{
		std::vector<glm::vec4> colors;
		Scene &scene;
		glm::vec2 size;
		std::string text;
		std::shared_ptr<TextView> textView;
		fonts::freetype::FreetypeFont &font;
		using OnClickHandler = std::function<void()>;
		OnClickHandler handler;
		Window::EventIdentifier mouseHoverID = 0;
		Window::EventIdentifier mousePressID = 0;
		inline static size_t buttonsCount = 0;
		Button(Window &window,
			   Scene &scene,
			   glm::vec3 position,
			   glm::vec3 rotation,
			   glm::vec3 scale,
			   glm::vec4 color,
			   glm::vec2 size,
			   std::string_view text,
			   fonts::freetype::FreetypeFont &font,
			   const OnClickHandler &handler,
			   const shaders::RuntimeConstants &constants = {},
			   std::string_view name = "");
		~Button() override;
		bool preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec2 size);
	};
}