#pragma once
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include "./TextView.hpp"
#include <anex/glm.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>

namespace anex::modules::gl::entities
{
	struct Button : GLEntity
	{
		std::vector<glm::vec4> colors;
		GLScene& scene;
		glm::vec2 size;
		std::string text;
		std::shared_ptr<TextView> textView;
		fonts::freetype::FreetypeFont& font;
    using OnClickHandler = std::function<void()>;
    OnClickHandler handler;
		GLWindow::EventIdentifier mouseHoverID = 0;
		GLWindow::EventIdentifier mousePressID = 0;
		inline static size_t buttonsCount = 0;
		Button(GLWindow& window,
							GLScene& scene,
							const glm::vec3& position,
							const glm::vec3& rotation,
							const glm::vec3& scale,
							const glm::vec4& color,
              const glm::vec2& size,
							std::string_view text,
							fonts::freetype::FreetypeFont& font,
							const OnClickHandler& handler,
							const shaders::RuntimeConstants& constants = {},
							std::string_view name = "");
		~Button() override;
		void preRender() override;
		void setColor(const glm::vec4& color);
		void setSize(const glm::vec2& size);
	};
}
