#pragma once
#include <zg/modules/gl/GLEntity.hpp>
#include <zg/modules/gl/GLScene.hpp>
#include <zg/modules/gl/GLWindow.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/modules/gl/fonts/freetype/Freetype.hpp>
#include <zg/modules/gl/ISizable.hpp>
#include <zg/strings/HookedConsole.hpp>
namespace zg::modules::gl::entities
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
		void hookedCallback(const std::vector<std::string> &lines);
		void showConsoleLines();
	};
}
