#pragma once
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include "./TextView.hpp"
#include <anex/glm.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>

namespace anex::modules::gl::entities
{
	struct PanelMenu : GLEntity
	{
		std::vector<glm::vec4> colors;
		GLScene& scene;
		glm::vec2 size;
		glm::vec4 color;
		fonts::freetype::FreetypeFont& font;
		std::string title;
		float width;
		float height;
		std::shared_ptr<TextView> titleTextView;
		inline static size_t panelMenusCount = 0;
		PanelMenu(GLWindow& window,
							GLScene& scene,
							const glm::vec3& position,
							const glm::vec3& rotation,
							const glm::vec3& scale,
							const glm::vec4& color,
							fonts::freetype::FreetypeFont& font,
							std::string_view title,
							float width,
							float height,
							const shaders::RuntimeConstants& constants = {},
							std::string_view name = "");
		void addItem(std::string_view name, GLEntity& entity);
		void preRender() override;
		void setColor(const glm::vec4& color);
		void setSize();
	};

	struct PanelItem : GLEntity
	{
		std::vector<glm::vec4> colors;
		GLScene& scene;
		glm::vec2 size;
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
							const glm::vec3& position,
							const glm::vec3& rotation,
							const glm::vec3& scale,
							const glm::vec4& color,
							std::string_view text,
							fonts::freetype::FreetypeFont& font,
							GLEntity& entity,
							float panelWidth,
							float indent,
							const shaders::RuntimeConstants& constants = {},
							std::string_view name = "");
		~PanelItem() override;
		void preRender() override;
		void setColor(const glm::vec4& color);
		void setSize(const glm::vec2& size);
	};
}
