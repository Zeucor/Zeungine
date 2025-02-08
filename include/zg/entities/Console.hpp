#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/interfaces/ISizable.hpp>
#include <zg/strings/HookedConsole.hpp>
namespace zg::entities
{
	struct Console : Entity, ISizable
	{
		std::vector<glm::vec4> colors;
		Scene &scene;
		glm::vec4 backgroundColor;
		fonts::freetype::FreetypeFont &font;
		float width;
		float height;
		std::vector<std::shared_ptr<TextView>> consoleTextViews;
		strings::HookedConsole hookedConsole;
		size_t currentIndex = 0;
		inline static size_t consolesCount = 0;
		Console(Window &window,
				Scene &scene,
				glm::vec3 position,
				glm::vec3 rotation,
				glm::vec3 scale,
				glm::vec4 backgroundColor,
				fonts::freetype::FreetypeFont &font,
				float width,
				float height,
				const shaders::RuntimeConstants &constants = {},
				std::string_view name = "");
		void preRender() override;
		void setBackgroundColor(glm::vec4 newBackgroundColor);
		void setSize(glm::vec3 newSize) override;
		void hookedCallback(const std::vector<std::string> &lines);
		void showConsoleLines();
	};
}