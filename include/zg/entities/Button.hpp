#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/fonts/freetype/Freetype.hpp>


namespace zg::entities
{
	using OnClickHandler = std::function<void()>;
	EntityCreateInfo ButtonFactory(
		glm::vec3 position,
		glm::quat rotation,
		glm::vec3 scale,
		glm::vec4 color,
		glm::vec2 size,
		std::string_view text,
		fonts::freetype::FreetypeFont &font,
		const OnClickHandler &handler,
		const shaders::RuntimeConstants &constants = {},
		std::string_view name = "");
	// struct Button : Entity
	// {
	// 	size_t getTypeID() override { return EntityTypeID<Button>::id; }
	// 	std::vector<glm::vec4> colors;
	// 	glm::vec2 size;
	// 	std::string text;
	// 	Entity* textView;
	// 	fonts::freetype::FreetypeFont &font;
	// 	OnClickHandler handler;
	// 	UniqueIdentifier mouseHoverID = 0;
	// 	UniqueIdentifier mousePressID = 0;
	// 	inline static size_t buttonsCount = 0;
	// 	// Button(Window &window,
	// 	// 	   );
	// 	~Button() override;
	// 	bool preRender() override;
	// 	void setColor(glm::vec4 color);
	// 	void setSize(glm::vec2 size);
	// };
}