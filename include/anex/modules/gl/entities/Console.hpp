#pragma once
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include "./TextView.hpp"
#include <anex/glm.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
#include <anex/modules/gl/ISizable.hpp>
#include <anex/strings/HookedConsole.hpp>
namespace anex::modules::gl::entities
{
	struct Console : GLEntity, ISizable
	{
		std::vector<glm::vec4> colors;
		GLScene& scene;
		glm::vec4 backgroundColor;
		fonts::freetype::FreetypeFont& font;
		float width;
		float height;
		std::vector<std::shared_ptr<TextView>> consoleTextViews;
		strings::HookedConsole hookedConsole;
		size_t currentIndex = 0;
		inline static size_t consolesCount = 0;
		Console(GLWindow& window,
						GLScene& scene,
						glm::vec3 position,
						glm::vec3 rotation,
						glm::vec3 scale,
						glm::vec4 backgroundColor,
						fonts::freetype::FreetypeFont& font,
						float width,
						float height,
						const shaders::RuntimeConstants& constants = {},
						std::string_view name = "");
		void preRender() override;
		void setBackgroundColor(glm::vec4 newBackgroundColor);
		void setSize(glm::vec3 newSize) override;
		void appendText(const std::string& stringToAppend);
		void hookedCallback(strings::HookedConsole& _hookedConsole, const std::string &currentLine, const std::vector<std::string> &thisLines);
		void showConsoleLines();
	};
}
