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
	struct PanelMenu : Entity, ISizable
	{
		size_t getTypeID() override { return EntityTypeID<PanelMenu>::id; }
		std::vector<glm::vec4> colors;
		glm::vec4 color;
		fonts::freetype::FreetypeFont &font;
		std::string title;
		float width;
		float height;
		std::shared_ptr<TextView> titleTextView;
		inline static size_t panelMenusCount = 0;
		PanelMenu(Window &window,
				  Scene &scene,
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
		void addPanelEntity(const std::shared_ptr<Entity> &entity, bool alignSizeX = true);
		void removePanelEntity(const std::shared_ptr<Entity> &entity);
		float getSizeYTotal();
		bool preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec3 newSize) override;
	};

	struct PanelItem : Entity, ISizable
	{
		size_t getTypeID() override { return EntityTypeID<PanelItem>::id; }
		std::vector<glm::vec4> colors;
		std::string text;
		std::shared_ptr<TextView> textView;
		fonts::freetype::FreetypeFont &font;
		Entity &entity;
		float panelWidth;
		float indent;
		UniqueIdentifier mouseHoverID = 0;
		UniqueIdentifier mousePressID = 0;
		inline static size_t panelItemsCount = 0;
		PanelItem(Window &window,
				  Scene &scene,
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
		~PanelItem() override;
		bool preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec3 newSize) override;
	};
}