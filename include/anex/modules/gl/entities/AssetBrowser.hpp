#pragma once
#include <anex/modules/gl/GLEntity.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
#include "./TextView.hpp"
#include <anex/glm.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
#include <anex/modules/gl/ISizable.hpp>
#include <FileWatch.hpp>
namespace anex::modules::gl::entities
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
		AssetBrowser(GLWindow& window,
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
