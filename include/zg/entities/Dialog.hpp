#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/fonts/freetype/Freetype.hpp>

namespace zg::entities
{
	struct Dialog : Entity
	{
		std::vector<glm::vec4> colors;
		Scene &scene;
		glm::vec2 size;
		glm::vec4 color;
		fonts::freetype::FreetypeFont &font;
		std::string title;
		float width;
		float height;
		float NDCHeight;
		float fontSize;
		std::shared_ptr<TextView> titleTextView;
		inline static size_t dialogsCount = 0;
		Dialog(Window &window,
			   Scene &scene,
			   glm::vec3 position,
			   glm::vec3 rotation,
			   glm::vec3 scale,
			   glm::vec4 color,
			   fonts::freetype::FreetypeFont &font,
			   std::string_view title,
			   float width,
			   float height,
			   const std::vector<std::shared_ptr<Entity>> &children = {},
			   const shaders::RuntimeConstants &constants = {},
			   std::string_view name = "");
		bool preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec2 newSize);
	};
}