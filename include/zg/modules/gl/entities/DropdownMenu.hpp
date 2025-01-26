#pragma once
#include <zg/modules/gl/GLEntity.hpp>
#include <zg/modules/gl/GLScene.hpp>
#include <zg/modules/gl/RenderWindow.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/modules/gl/fonts/freetype/Freetype.hpp>
namespace zg::modules::gl::entities
{
	struct DropdownMenu : GLEntity
	{
		std::vector<glm::vec4> colors;
    GLScene &scene;
		glm::vec2 size;
    using OptionPressHandler = std::function<void()>;
		fonts::freetype::FreetypeFont &font;
		inline static size_t dropdownMenusCount = 0;
		DropdownMenu(RenderWindow &window,
								 GLScene &scene,
								 glm::vec3 position,
								 glm::vec3 rotation,
								 glm::vec3 scale,
								 glm::vec4 color,
								 fonts::freetype::FreetypeFont &font,
								 const shaders::RuntimeConstants &constants = {},
								 std::string_view name = "");
    void addOption(std::string_view name, const OptionPressHandler &handler);
		void preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec2 size);
	};
	struct DropdownItem : GLEntity
	{
		std::vector<glm::vec4> colors;
		GLScene &scene;
		glm::vec2 size;
		std::string text;
    DropdownMenu::OptionPressHandler handler;
    std::shared_ptr<TextView> textView;
		fonts::freetype::FreetypeFont &font;
		RenderWindow::EventIdentifier mouseHoverID = 0;
		RenderWindow::EventIdentifier mousePressID = 0;
		inline static size_t dropdownItemsCount = 0;
		DropdownItem(RenderWindow &window,
								 GLScene &scene,
								 glm::vec3 position,
								 glm::vec3 rotation,
								 glm::vec3 scale,
								 glm::vec4 color,
								 std::string_view text,
								 const DropdownMenu::OptionPressHandler &handler,
								 fonts::freetype::FreetypeFont &font,
								 const shaders::RuntimeConstants &constants = {},
								 std::string_view name = "");
		~DropdownItem() override;
		void preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec2 size);
	};
}