#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/interfaces/ISizable.hpp>


namespace zg::entities
{
	EntityCreateInfo PanelMenuFactory(
		glm::vec3 position,
		glm::quat rotation,
		glm::vec3 scale,
		glm::vec4 color,
		fonts::freetype::FreetypeFont &font,
		std::string_view title,
		float width,
		float height,
		const shaders::RuntimeConstants &constants = {},
		std::string_view name = "");
	// struct PanelMenu : Entity, ISizable
	// {
	// 	size_t getTypeID() override { return EntityTypeID<PanelMenu>::id; }
	// 	std::vector<glm::vec4> colors;
	// 	glm::vec4 color;
	// 	fonts::freetype::FreetypeFont &font;
	// 	std::string title;
	// 	float width;
	// 	float height;
	// 	Entity* titleTextView;
	// 	inline static size_t panelMenusCount = 0;
	// 	PanelMenu(Window &window,
	// 			  );
	// 	void addPanelEntity(const Entity &entity, bool alignSizeX = true);
	// 	void removePanelEntity(const Entity &entity);
	// 	float getSizeYTotal();
	// 	bool preRender() override;
	// 	void setColor(glm::vec4 color);
	// 	void setSize(glm::vec3 newSize) override;
	// };
	EntityCreateInfo PanelItemFactory(
		glm::vec3 position,
		glm::quat rotation,
		glm::vec3 scale,
		glm::vec4 color,
		std::string_view text,
		fonts::freetype::FreetypeFont &font,
		Entity &entity,
		float panelWidth,
		float indent,
		const shaders::RuntimeConstants &constants = {},
		std::string_view name = "");
	// struct PanelItem : Entity, ISizable
	// {
	// 	size_t getTypeID() override { return EntityTypeID<PanelItem>::id; }
	// 	std::vector<glm::vec4> colors;
	// 	std::string text;
	// 	Entity* textView;
	// 	fonts::freetype::FreetypeFont &font;
	// 	Entity &entity;
	// 	float panelWidth;
	// 	float indent;
	// 	UniqueIdentifier mouseHoverID = 0;
	// 	UniqueIdentifier mousePressID = 0;
	// 	inline static size_t panelItemsCount = 0;
	// 	PanelItem(Window &window,
	// 			  );
	// 	~PanelItem() override;
	// 	bool preRender() override;
	// 	void setColor(glm::vec4 color);
	// 	void setSize(glm::vec3 newSize) override;
	// };
}