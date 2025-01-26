#pragma once
#include <zg/modules/gl/GLEntity.hpp>
#include <zg/modules/gl/GLScene.hpp>
#include <zg/modules/gl/RenderWindow.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/modules/gl/fonts/freetype/Freetype.hpp>
#include <zg/modules/gl/ISizable.hpp>
#include <FileWatch.hpp>
namespace zg::modules::gl::entities
{
	struct AssetBrowser : GLEntity, ISizable
	{
		std::vector<glm::vec4> colors;
		GLScene& scene;
		glm::vec4 backgroundColor;
		fonts::freetype::FreetypeFont& font;
		float width;
		float height;
		std::string_view projectDirectory;
		filewatch::FileWatch<std::string> projectDirectoryWatch;
		size_t currentIndex = 0;
		inline static size_t assetBrowsersCount = 0;
		AssetBrowser(RenderWindow& window,
								 GLScene& scene,
								 glm::vec3 position,
								 glm::vec3 rotation,
								 glm::vec3 scale,
								 glm::vec4 backgroundColor,
								 fonts::freetype::FreetypeFont& font,
								 float width,
								 float height,
								 std::string_view projectDirectory,
								 const shaders::RuntimeConstants& constants = {},
								 std::string_view name = "");
		void preRender() override;
		void setBackgroundColor(glm::vec4 newBackgroundColor);
		void setSize(glm::vec3 newSize) override;
	};
}
