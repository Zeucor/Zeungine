#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/filesystem/DirectoryWatcher.hpp>
#include <zg/fonts/freetype/Freetype.hpp>
#include <zg/glm.hpp>
#include <zg/interfaces/ISizable.hpp>
#include "./TextView.hpp"
namespace zg::entities
{
	struct Asset : Entity, ISizable
	{
		Scene& scene;
		std::filesystem::path path;
		fonts::freetype::FreetypeFont& font;
		inline static size_t assetsCount = 0;
		Asset(Window& window, Scene& scene, const std::filesystem::path& path, fonts::freetype::FreetypeFont& font);
		bool preRender() override;
	};
	struct Breadcrumbs : Entity, ISizable
	{
		Scene& scene;
		std::vector<glm::vec4> colors;
		glm::vec4 backgroundColor;
		float width;
		float fontSize;
		float lineHeight = 0;
		fonts::freetype::FreetypeFont& font;
		std::filesystem::path rootPath;
		std::filesystem::path currentPath;
		std::vector<std::string> pathSplit;
		std::vector<std::shared_ptr<textures::Texture>> textures;
		std::vector<std::shared_ptr<entities::Plane>> texturePlanes;
		inline static size_t breadcrumbsCount = 0;
		Breadcrumbs(Window& window, Scene& scene, float width, fonts::freetype::FreetypeFont& font, glm::vec3 position,
								const std::filesystem::path& rootPath);
		bool preRender() override;
		void setCurrentPath(const std::filesystem::path& currentPath);
		void setSize(glm::vec3 newSize) override;
	};
	struct AssetGrid : Entity, ISizable
	{
		Scene& scene;
		std::vector<glm::vec4> colors;
		glm::vec4 backgroundColor;
		float width;
		float height;
		inline static size_t assetGridsCount = 0;
		AssetGrid(Window& window, Scene& scene, float width, float height, glm::vec3 position);
		bool preRender() override;
		size_t addAsset(const std::shared_ptr<Asset>& asset);
		void setSize(glm::vec3 newSize) override;
	};
	struct AssetBrowser : Entity, ISizable
	{
		std::vector<glm::vec4> colors;
		glm::vec4 backgroundColor;
		Scene& scene;
		fonts::freetype::FreetypeFont& font;
		float width;
		float height;
		std::vector<std::filesystem::path> excludePaths;
		std::filesystem::path projectDirectory;
		filesystem::DirectoryWatcher directoryWatcher;
		std::filesystem::path currentDirectory;
		size_t currentIndex = 0;
		std::shared_ptr<Breadcrumbs> breadcrumbs;
		std::shared_ptr<AssetGrid> assetGrid;
		inline static size_t assetBrowsersCount = 0;
		AssetBrowser(Window& window, Scene& scene, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
								 glm::vec4 backgroundColor, fonts::freetype::FreetypeFont& font, float width, float height,
								 std::filesystem::path projectDirectory, const shaders::RuntimeConstants& constants = {},
								 std::string_view name = "");
		void update() override;
		bool preRender() override;
		void setBackgroundColor(glm::vec4 newBackgroundColor);
		void setSize(glm::vec3 newSize) override;
	};
} // namespace zg::entities
