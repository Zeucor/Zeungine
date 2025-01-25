#pragma once
#include <zg/modules/gl/GLEntity.hpp>
#include <zg/modules/gl/GLScene.hpp>
#include <zg/modules/gl/GLWindow.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/modules/gl/fonts/freetype/Freetype.hpp>
#include <zg/modules/gl/ISizable.hpp>

namespace zg::modules::gl::entities
{
	struct PanelMenu : GLEntity, ISizable
	{
		std::vector<glm::vec4> colors;
		GLScene& scene;
		glm::vec4 color;
		fonts::freetype::FreetypeFont& font;
		std::string title;
		float width;
		float height;
		std::shared_ptr<TextView> titleTextView;
		inline static size_t panelMenusCount = 0;
		PanelMenu(GLWindow& window,
							GLScene& scene,
							glm::vec3 position,
							glm::vec3 rotation,
							glm::vec3 scale,
							glm::vec4 color,
							fonts::freetype::FreetypeFont& font,
							std::string_view title,
							float width,
							float height,
							const shaders::RuntimeConstants& constants = {},
							std::string_view name = "");
		void addPanelEntity(const std::shared_ptr<GLEntity> &entity, bool alignSizeX = true);
		void removePanelEntity(const std::shared_ptr<GLEntity> &entity);
		float getSizeYTotal();
		void preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec3 newSize) override;
	};

	struct PanelItem : GLEntity, ISizable
	{
		std::vector<glm::vec4> colors;
		GLScene& scene;
		std::string text;
		std::shared_ptr<TextView> textView;
		fonts::freetype::FreetypeFont& font;
		GLEntity& entity;
		float panelWidth;
		float indent;
		GLWindow::EventIdentifier mouseHoverID = 0;
		GLWindow::EventIdentifier mousePressID = 0;
		inline static size_t panelItemsCount = 0;
		PanelItem(GLWindow& window,
							GLScene& scene,
							glm::vec3 position,
							glm::vec3 rotation,
							glm::vec3 scale,
							glm::vec4 color,
							std::string_view text,
							fonts::freetype::FreetypeFont& font,
							GLEntity& entity,
							float panelWidth,
							float indent,
							const shaders::RuntimeConstants& constants = {},
							std::string_view name = "");
		~PanelItem() override;
		void preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec3 newSize) override;
	};
}
