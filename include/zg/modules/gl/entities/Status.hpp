#pragma once
#include <zg/modules/gl/GLEntity.hpp>
#include <zg/modules/gl/GLScene.hpp>
#include <zg/modules/gl/RenderWindow.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/modules/gl/fonts/freetype/Freetype.hpp>

namespace zg::modules::gl::entities
{
	struct Tab;
	struct Status : GLEntity
	{
		std::vector<glm::vec4> colors;
		GLScene& scene;
		glm::vec2 size;
		glm::vec4 color;
		fonts::freetype::FreetypeFont& font;
		std::string text;
		std::shared_ptr<TextView> textView;
		float width;
		float height;
		float NDCHeight;
		inline static size_t statusesCount = 0;
		using TabClickHandler = std::function<void()>;
		Status(RenderWindow& window,
					 GLScene& scene,
					 glm::vec3 position,
					 glm::vec3 rotation,
					 glm::vec3 scale,
					 glm::vec4 color,
					 fonts::freetype::FreetypeFont& font,
					 float width,
					 float height,
					 std::string_view text = "",
					 const shaders::RuntimeConstants& constants = {},
					 std::string_view name = "");
		void preRender() override;
		void setColor(glm::vec4 color);
		void setSize();
		void setText(std::string_view text);
		void setTextColor(glm::vec4 newTextColor);
	};
}
