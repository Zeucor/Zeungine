#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/fonts/freetype/Freetype.hpp>


namespace zg::entities
{
	EntityCreateInfo DialogFactory(
				   glm::vec3 position,
				   glm::quat rotation,
				   glm::vec3 scale,
				   glm::vec4 color,
				   fonts::freetype::FreetypeFont &font,
				   std::string_view title,
				   float width,
				   float height,
				   const std::vector<EntityCreateInfo> &childrenCreateInfos = {},
				   const shaders::RuntimeConstants &constants = {},
				   std::string_view name = "");
	// struct Dialog : Entity
	// {
	// 	size_t getTypeID() override { return EntityTypeID<Dialog>::id; }
	// 	std::vector<glm::vec4> colors;
	// 	glm::vec2 size;
	// 	glm::vec4 color;
	// 	fonts::freetype::FreetypeFont &font;
	// 	std::string title;
	// 	float width;
	// 	float height;
	// 	float NDCHeight;
	// 	float fontSize;
	// 	Entity* titleTextView;
	// 	inline static size_t dialogsCount = 0;
	// 	Dialog(Window &window,
	// 		   Scene &scene,
	// 		   glm::vec3 position,
	// 		   glm::quat rotation,
	// 		   glm::vec3 scale,
	// 		   glm::vec4 color,
	// 		   fonts::freetype::FreetypeFont &font,
	// 		   std::string_view title,
	// 		   float width,
	// 		   float height,
	// 		   const std::vector<EntityCreateInfo> &childrenCreateInfos = {},
	// 		   const shaders::RuntimeConstants &constants = {},
	// 		   std::string_view name = "");
	// 	bool preRender() override;
	// 	void setColor(glm::vec4 color);
	// 	void setSize(glm::vec2 newSize);
	// };
}