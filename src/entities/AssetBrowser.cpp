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
													projectDirectory(projectDirectory),
													projectDirectoryWatch(std::string(projectDirectory), [&](const auto &filePath, const auto eventType)
																		  {
		if (filePath.find("build") == 0)
			return;
		std::cout << filePath << " : ";
		switch (eventType)
		{
		case filewatch::Event::added:
			std::cout << "The file was added to the directory.";
			break;
		case filewatch::Event::removed:
			std::cout << "The file was removed from the directory.";
			break;
		case filewatch::Event::modified:
			std::cout << "The file was modified. This can be a change in the time stamp or attributes.";
			break;
		case filewatch::Event::renamed_old:
			std::cout << "The file was renamed and this is the old name.";
			break;
		case filewatch::Event::renamed_new:
			std::cout << "The file was renamed and this is the new name.";
			break;
		default:
			std::cout << "Unknown event type.";
		};
		std::cout << std::endl; })
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
void AssetBrowser::preRender()
{
	const auto &model = getModelMatrix();
	shader->bind(*this);
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, scene.view.matrix);
	shader->setBlock("Projection", *this, scene.projection.matrix);
	shader->setBlock("CameraPosition", *this, scene.view.position, 16);
	shader->unbind();
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