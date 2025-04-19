#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/fonts/freetype/Freetype.hpp>

namespace zg::entities
{
	struct Tab;
	EntityCreateInfo StatusTextFactory(
		glm::vec3 position,
		glm::quat rotation,
		glm::vec3 scale,
		glm::vec4 color,
		fonts::freetype::FreetypeFont &font,
		float width,
		float height,
		std::string_view text = "",
		const shaders::RuntimeConstants &constants = {},
		std::string_view name = "");
	// struct StatusText : Entity
	// {
	// 	size_t getTypeID() override { return EntityTypeID<StatusText>::id; }
	// 	std::vector<glm::vec4> colors;
	// 	glm::vec2 size;
	// 	glm::vec4 color;
	// 	fonts::freetype::FreetypeFont &font;
	// 	std::string text;
	// 	Entity* textView;
	// 	float width;
	// 	float height;
	// 	float NDCHeight;
	// 	inline static size_t statusTextsCount = 0;
	// 	using TabClickHandler = std::function<void()>;
	// 	StatusText(Window &window,
	// 			   );
	// 	bool preRender() override;
	// 	void setColor(glm::vec4 color);
	// 	void setSize();
	// 	void setText(std::string_view text);
	// 	void setTextColor(glm::vec4 newTextColor);
	// };
}