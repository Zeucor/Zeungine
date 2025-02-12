#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include "./TextView.hpp"
#include <zg/glm.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/interfaces/ISizable.hpp>
namespace zg::entities
{
	struct AssetBrowser : Entity, ISizable
	{
		std::vector<glm::vec4> colors;
		Scene &scene;
		glm::vec4 backgroundColor;
		fonts::freetype::FreetypeFont &font;
		float width;
		float height;
		std::string_view projectDirectory;
		size_t currentIndex = 0;
		inline static size_t assetBrowsersCount = 0;
		AssetBrowser(Window &window,
					 Scene &scene,
					 glm::vec3 position,
					 glm::vec3 rotation,
					 glm::vec3 scale,
					 glm::vec4 backgroundColor,
					 fonts::freetype::FreetypeFont &font,
					 float width,
					 float height,
					 std::string_view projectDirectory,
					 const shaders::RuntimeConstants &constants = {},
					 std::string_view name = "");
		bool preRender() override;
		void setBackgroundColor(glm::vec4 newBackgroundColor);
		void setSize(glm::vec3 newSize) override;
	};
}