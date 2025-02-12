#include <zg/entities/AssetBrowser.hpp>
#include <zg/utilities.hpp>
#include <iostream>
using namespace zg::entities;
AssetBrowser::AssetBrowser(zg::Window &window,
						   zg::Scene &scene,
						   glm::vec3 position,
						   glm::vec3 rotation,
						   glm::vec3 scale,
						   glm::vec4 backgroundColor,
						   fonts::freetype::FreetypeFont &font,
						   float width,
						   float height,
						   std::string_view projectDirectory,
						   const zg::shaders::RuntimeConstants &constants,
						   std::string_view name) : zg::Entity(window,
															   zg::mergeVectors<std::string_view>({{"Color", "Position",
																									"View", "Projection", "Model", "CameraPosition"}},
																								  constants),
															   6,
															   {0, 1, 2, 2, 3, 0},
															   4,
															   {{0, -0, 0}, {0, -0, 0}, {0, 0, 0}, {0, 0, 0}},
															   position,
															   rotation,
															   scale,
															   name.empty() ? "AssetBrowser " + std::to_string(++assetBrowsersCount) : name),
													scene(scene),
													backgroundColor(backgroundColor),
													font(font),
													width(width),
													height(height),
													projectDirectory(projectDirectory)
{
	updateIndices(indices);
	setBackgroundColor(backgroundColor);
	updateElements("Position", positions);
	AssetBrowser::setSize(glm::vec3(0));
	this->addMousePressHandler(3, [&](auto pressed)
							   {
		if (pressed)
		{
			currentIndex++;
		} });
	this->addMousePressHandler(4, [&](auto pressed)
							   {
		if (pressed && currentIndex > 0)
		{
			currentIndex--;
		} });
};
bool AssetBrowser::preRender()
{
	const auto &model = getModelMatrix();
	shader->bind(*this);
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, scene.view.matrix);
	shader->setBlock("Projection", *this, scene.projection.matrix);
	shader->setBlock("CameraPosition", *this, scene.view.position, 16);
	shader->unbind();
	return true;
};
void AssetBrowser::setBackgroundColor(glm::vec4 newBackgroundColor)
{
	backgroundColor = newBackgroundColor;
	colors = {backgroundColor, backgroundColor, backgroundColor, backgroundColor};
	updateElements("Color", colors);
}
void AssetBrowser::setSize(glm::vec3 newSize)
{
	glm::vec3 actualNewSize(width / window.windowWidth / 0.5, height / window.windowHeight / 0.5, 0);
	positions = {
		{0, -actualNewSize.y, 0}, {actualNewSize.x, -actualNewSize.y, 0}, {actualNewSize.x, 0, 0}, {0, 0, 0}};
	updateElements("Position", positions);
	size = actualNewSize;
}