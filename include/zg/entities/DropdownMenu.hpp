#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
namespace zg::entities
{
	struct DropdownMenu : Entity
	{
		std::vector<glm::vec4> colors;
		Scene &scene;
		glm::vec2 size;
		using OptionPressHandler = std::function<void()>;
		fonts::freetype::FreetypeFont &font;
		inline static size_t dropdownMenusCount = 0;
		DropdownMenu(Window &window,
					 Scene &scene,
					 glm::vec3 position,
					 glm::vec3 rotation,
					 glm::vec3 scale,
					 glm::vec4 color,
					 fonts::freetype::FreetypeFont &font,
					 const shaders::RuntimeConstants &constants = {},
					 std::string_view name = "");
		void addOption(std::string_view name, const OptionPressHandler &handler);
		bool preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec2 size);
	};
	struct DropdownItem : Entity
	{
		std::vector<glm::vec4> colors;
		Scene &scene;
		glm::vec2 size;
		std::string text;
		DropdownMenu::OptionPressHandler handler;
		std::shared_ptr<TextView> textView;
		fonts::freetype::FreetypeFont &font;
		Window::EventIdentifier mouseHoverID = 0;
		Window::EventIdentifier mousePressID = 0;
		inline static size_t dropdownItemsCount = 0;
		DropdownItem(Window &window,
					 Scene &scene,
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
		bool preRender() override;
		void setColor(glm::vec4 color);
		void setSize(glm::vec2 size);
	};
}