#pragma once
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include "./TextView.hpp"
#include <anex/glm.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
namespace anex::modules::gl::entities
{
	struct DropdownMenu : GLEntity
	{
		std::vector<glm::vec4> colors;
    GLScene &scene;
		glm::vec2 size;
    using OptionPressHandler = std::function<void()>;
		fonts::freetype::FreetypeFont &font;
		inline static size_t dropdownMenusCount = 0;
		DropdownMenu(GLWindow &window,
								 GLScene &scene,
								 const glm::vec3 &position,
								 const glm::vec3 &rotation,
								 const glm::vec3 &scale,
								 const glm::vec4 &color,
								 fonts::freetype::FreetypeFont &font,
								 const shaders::RuntimeConstants &constants = {},
								 std::string_view name = "");
    void addOption(std::string_view name, const OptionPressHandler &handler);
		void preRender() override;
		void setColor(const glm::vec4 &color);
		void setSize(const glm::vec2 &size);
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
		GLWindow::EventIdentifier mouseHoverID = 0;
		GLWindow::EventIdentifier mousePressID = 0;
		inline static size_t dropdownItemsCount = 0;
		DropdownItem(GLWindow &window,
								 GLScene &scene,
								 const glm::vec3 &position,
								 const glm::vec3 &rotation,
								 const glm::vec3 &scale,
								 const glm::vec4 &color,
								 std::string_view text,
								 const DropdownMenu::OptionPressHandler &handler,
								 fonts::freetype::FreetypeFont &font,
								 const shaders::RuntimeConstants &constants = {},
								 std::string_view name = "");
		~DropdownItem() override;
		void preRender() override;
		void setColor(const glm::vec4 &color);
		void setSize(const glm::vec2 &size);
	};
}