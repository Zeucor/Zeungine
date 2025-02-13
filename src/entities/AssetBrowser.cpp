#include <iostream>
#include <zg/entities/AssetBrowser.hpp>
#include <zg/filesystem/Directory.hpp>
#include <zg/utilities.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/images/SVGRasterize.hpp>
using namespace zg::entities;
Asset::Asset(Window& window, Scene& scene, const std::filesystem::path& path, fonts::freetype::FreetypeFont& font) :
		Entity(window, {"Color", "Position", "View", "Projection", "Model", "CameraPosition"}, 6, {0, 1, 2, 2, 3, 0}, 4,
					 {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, glm::vec3(0), glm::vec3(0), glm::vec3(1),
					 "Asset " + std::to_string(++assetsCount)),
		scene(scene),
		path(path), font(font)
{
}
bool Asset::preRender()
{
	const auto& model = getModelMatrix();
	shader->bind(*this);
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, scene.view.matrix);
	shader->setBlock("Projection", *this, scene.projection.matrix);
	shader->setBlock("CameraPosition", *this, scene.view.position, 16);
	shader->unbind();
	return true;
}
Breadcrumbs::Breadcrumbs(Window& window, Scene& scene, float width, fonts::freetype::FreetypeFont& font,
												 glm::vec3 position, const std::filesystem::path& rootPath) :
		Entity(window, {"Color", "Position", "View", "Projection", "Model", "CameraPosition"}, 6, {0, 1, 2, 2, 3, 0}, 4,
					 {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, position, glm::vec3(0), glm::vec3(1),
					 "Breadcrumb " + std::to_string(++breadcrumbsCount)),
		scene(scene),
		width(width),
		fontSize(window.windowHeight / 32),
		font(font), rootPath(rootPath)
{
	updateIndices(indices);
	backgroundColor = {0.4, 0.4, 0.4, 1};
	colors = {backgroundColor, backgroundColor, backgroundColor, backgroundColor};
	updateElements("Color", colors);
	font.stringSize("T", fontSize, lineHeight, glm::vec2(0));
	setSize(glm::vec3(0));
	setCurrentPath(rootPath / "src" / "test" / "test2");
}
bool Breadcrumbs::preRender()
{
	const auto& model = getModelMatrix();
	shader->bind(*this);
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, scene.view.matrix);
	shader->setBlock("Projection", *this, scene.projection.matrix);
	shader->setBlock("CameraPosition", *this, scene.view.position, 16);
	shader->unbind();
	return true;
}
void Breadcrumbs::setCurrentPath(const std::filesystem::path& currentPath)
{
	this->currentPath = currentPath;
	std::vector<std::string> items;
	items.push_back(rootPath.filename().string());
	std::filesystem::path relativePath = std::filesystem::relative(this->currentPath, rootPath);
	for (const auto& part : relativePath)
		if (part != ".")
			items.push_back(part.string());
	pathSplit = items;
	textures.clear();
	for (auto& texturePlane : texturePlanes)
		removeChild(texturePlane->ID);
	texturePlanes.clear();
	auto pathSplitSize = pathSplit.size();
	glm::vec3 currentPosition(0, (-lineHeight / 2) / window.windowHeight / 0.5, 0.1);
	auto editorDirectoryPath = filesystem::File::getProgramDirectoryPath();
	auto addChevron = [&]
	{
		filesystem::File arrowFile(editorDirectoryPath / "icons" / "Remix" / "Arrows" / "arrow-drop-right-line.svg", enums::EFileLocation::Absolute, "r");
		float arrowSize = lineHeight;
		auto arrowBitmap = images::SVGRasterize(arrowFile, glm::ivec2(arrowSize, arrowSize));
		auto arrowTexture = std::make_shared<textures::Texture>(
			window,
			glm::ivec4(arrowSize, arrowSize, 1, 0),
			(void*)arrowBitmap.get()
		);
		arrowTexture->bind();
		textures.push_back(arrowTexture);
		auto& arrowTextureSize = arrowTexture->size;
		currentPosition.x += arrowTextureSize.x / 2 / window.windowWidth / 0.5;
		auto arrowPlane = std::make_shared<Plane>(
			window,
			scene,
			currentPosition,
			glm::vec3(0),
			glm::vec3(1),
			glm::vec2(arrowTextureSize.x / window.windowWidth / 0.5, arrowTextureSize.y / window.windowHeight / 0.5),
			*arrowTexture
		);
		texturePlanes.push_back(arrowPlane);
		currentPosition.x += arrowPlane->size.x / 2;
		addChild(arrowPlane);
	};
	addChevron();
	for (int32_t i = 0; i < pathSplitSize; ++i)
	{
		auto& part = pathSplit[i];
		auto partTextSize = font.stringSize(part, fontSize, lineHeight, glm::vec2(0));
		std::shared_ptr<textures::Texture> partTexture;
		glm::vec3 cursorPosition;
		font.stringToTexture(part, glm::vec4(1), fontSize, lineHeight, partTextSize, partTexture, 0, cursorPosition);
		partTexture->bind();
		auto& partTextureSize = partTexture->size;
		textures.push_back(partTexture);
		currentPosition.x += partTextureSize.x / 2 / window.windowWidth / 0.5;
		auto partPlane = std::make_shared<Plane>(
			window,
			scene,
			currentPosition,
			glm::vec3(0),
			glm::vec3(1),
			glm::vec2(partTextureSize.x / window.windowWidth / 0.5, partTextureSize.y / window.windowHeight / 0.5),
			*partTexture
		);
		texturePlanes.push_back(partPlane);
		currentPosition.x += partPlane->size.x / 2;
		addChild(partPlane);
		if (i < pathSplitSize - 1)
			addChevron();
	}
}
void Breadcrumbs::setSize(glm::vec3 newSize)
{
	glm::vec3 actualNewSize(width / window.windowWidth / 0.5, lineHeight / window.windowHeight / 0.5, 0);
	positions = {{0, -actualNewSize.y, 0}, {actualNewSize.x, -actualNewSize.y, 0}, {actualNewSize.x, 0, 0}, {0, 0, 0}};
	updateElements("Position", positions);
	size = actualNewSize;
}
AssetGrid::AssetGrid(Window& window, Scene& scene, float width, float height, glm::vec3 position):
	Entity(window, {"Color", "Position", "View", "Projection", "Model", "CameraPosition"}, 6, {0, 1, 2, 2, 3, 0}, 4,
					 {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, position, glm::vec3(0), glm::vec3(1),
					 "AssetGrid " + std::to_string(++assetGridsCount)),
	scene(scene),
	width(width),
	height(height)
{
	updateIndices(indices);
	backgroundColor = {0.3, 0.3, 0.3, 1};
	colors = {backgroundColor, backgroundColor, backgroundColor, backgroundColor};
	updateElements("Color", colors);
	setSize(glm::vec3(0));
}
bool AssetGrid::preRender()
{
	const auto& model = getModelMatrix();
	shader->bind(*this);
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, scene.view.matrix);
	shader->setBlock("Projection", *this, scene.projection.matrix);
	shader->setBlock("CameraPosition", *this, scene.view.position, 16);
	shader->unbind();
	return true;
}
size_t AssetGrid::addAsset(const std::shared_ptr<Asset>& asset)
{
	addChild(asset);
	return asset->ID;
}
void AssetGrid::setSize(glm::vec3 newSize)
{
	glm::vec3 actualNewSize(width, height, 0);
	positions = {{0, -actualNewSize.y, 0}, {actualNewSize.x, -actualNewSize.y, 0}, {actualNewSize.x, 0, 0}, {0, 0, 0}};
	updateElements("Position", positions);
	size = actualNewSize;
}
AssetBrowser::AssetBrowser(zg::Window& window, zg::Scene& scene, glm::vec3 position, glm::vec3 rotation,
													 glm::vec3 scale, glm::vec4 backgroundColor, fonts::freetype::FreetypeFont& font, float width,
													 float height, std::filesystem::path projectDirectory,
													 const zg::shaders::RuntimeConstants& constants, std::string_view name) :
		zg::Entity(window,
							 zg::mergeVectors<std::string_view>(
								 {{"Color", "Position", "View", "Projection", "Model", "CameraPosition"}}, constants),
							 6, {0, 1, 2, 2, 3, 0}, 4, {{0, -0, 0}, {0, -0, 0}, {0, 0, 0}, {0, 0, 0}}, position, rotation, scale,
							 name.empty() ? "AssetBrowser " + std::to_string(++assetBrowsersCount) : name),
		scene(scene), backgroundColor(backgroundColor), font(font), width(width), height(height),
		excludePaths({projectDirectory / "build", projectDirectory / ".vscode"}), projectDirectory(projectDirectory),
		directoryWatcher(projectDirectory, excludePaths), currentDirectory(projectDirectory)
{
	updateIndices(indices);
	setBackgroundColor(backgroundColor);
	updateElements("Position", positions);
	AssetBrowser::setSize(glm::vec3(0));
	this->addMousePressHandler(3,
														 [&](auto pressed)
														 {
															 if (pressed)
															 {
																 currentIndex++;
															 }
														 });
	this->addMousePressHandler(4,
														 [&](auto pressed)
														 {
															 if (pressed && currentIndex > 0)
															 {
																 currentIndex--;
															 }
														 });
	//
	filesystem::Directory directory(projectDirectory);
	auto recursiveFileMap = directory.getRecursiveFileMap();
	for (auto& file : recursiveFileMap)
	{
		auto& filePath = file.second;
		if (!filesystem::DirectoryWatcher::isExcluded(excludePaths, filePath))
			if (std::filesystem::is_directory(filePath))
				std::cout << filePath << std::endl;
	}
	// breadcrumbs
	breadcrumbs = std::make_shared<Breadcrumbs>(
		window,
		scene,
		width,
		font,
		glm::vec3(0, 0, 0.1),
		projectDirectory
	);
	addChild(breadcrumbs);
	// asset grid
	assetGrid = std::make_shared<AssetGrid>(
		window,
		scene,
		width / window.windowWidth / 0.5,
		(height - breadcrumbs->lineHeight) / window.windowHeight / 0.5,
		glm::vec3(0, -breadcrumbs->lineHeight / window.windowHeight / 0.5, 0.1)
	);
	addChild(assetGrid);
};
void AssetBrowser::update()
{
	auto changes = directoryWatcher.update();
	for (auto& changePair : changes)
	{
	}
}
bool AssetBrowser::preRender()
{
	const auto& model = getModelMatrix();
	shader->bind(*this);
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, scene.view.matrix);
	shader->setBlock("Projection", *this, scene.projection.matrix);
	shader->setBlock("CameraPosition", *this, scene.view.position, 16);
	shader->unbind();
	return true;
}
void AssetBrowser::setBackgroundColor(glm::vec4 newBackgroundColor)
{
	backgroundColor = newBackgroundColor;
	colors = {backgroundColor, backgroundColor, backgroundColor, backgroundColor};
	updateElements("Color", colors);
}
void AssetBrowser::setSize(glm::vec3 newSize)
{
	glm::vec3 actualNewSize(width / window.windowWidth / 0.5, height / window.windowHeight / 0.5, 0);
	positions = {{0, -actualNewSize.y, 0}, {actualNewSize.x, -actualNewSize.y, 0}, {actualNewSize.x, 0, 0}, {0, 0, 0}};
	updateElements("Position", positions);
	size = actualNewSize;
}
