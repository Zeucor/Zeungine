#pragma once
#include <zg/modules/gl/GLEntity.hpp>
#include <zg/modules/gl/GLScene.hpp>
#include <zg/modules/gl/RenderWindow.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/modules/gl/fonts/freetype/Freetype.hpp>

namespace zg::modules::gl::entities
{
	struct Dialog : GLEntity
	{
		std::vector<glm::vec4> colors;
		GLScene& scene;
		glm::vec2 size;
		glm::vec4 color;
		fonts::freetype::FreetypeFont& font;
		std::string title;
		float width;
		float height;
		float NDCHeight;
		float fontSize;
		std::shared_ptr<TextView> titleTextView;
		inline static size_t dialogsCount = 0;
		Dialog(RenderWindow& window,
							GLScene& scene,
							glm::vec3 position,
							glm::vec3 rotation,
							glm::vec3 scale,
							glm::vec4 color,
							fonts::freetype::FreetypeFont& font,
							std::string_view title,
							float width,
							float height,
              const std::vector<std::shared_ptr<GLEntity>> &children = {},
							const shaders::RuntimeConstants& constants = {},
							std::string_view name = "");
		void preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec2 newSize);
	};
}
