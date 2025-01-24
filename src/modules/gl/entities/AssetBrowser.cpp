#include <anex/modules/gl/entities/AssetBrowser.hpp>
#include <anex/utilities.hpp>
#include <iostream>
using namespace anex::modules::gl::entities;
AssetBrowser::AssetBrowser(anex::modules::gl::GLWindow &window,
							             anex::modules::gl::GLScene &scene,
							             glm::vec3 position,
							             glm::vec3 rotation,
							             glm::vec3 scale,
							             glm::vec4 backgroundColor,
							             fonts::freetype::FreetypeFont &font,
													 float width,
													 float height,
							             const anex::modules::gl::shaders::RuntimeConstants &constants,
			                     std::string_view name):
	anex::modules::gl::GLEntity(
		window,
		anex::mergeVectors<std::string_view>({
			{
				"Color", "Position",
				"View", "Projection", "Model", "CameraPosition"
			}
		}, constants),
		6,
		{0, 1, 2,  2, 3, 0},
    4,
		{
			{ 0, -0, 0 }, { 0, -0, 0 }, { 0, 0, 0 }, { 0, 0, 0 }
		},
		position,
		rotation,
		scale,
		name.empty() ? "AssetBrowser " + std::to_string(++assetBrowsersCount) : name
	),
	scene(scene),
  backgroundColor(backgroundColor),
	font(font),
	width(width),
	height(height)
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
		}
	});
	this->addMousePressHandler(4, [&](auto pressed)
	{
		if (pressed && currentIndex > 0)
		{
			currentIndex--;
		}
	});
};
void AssetBrowser::preRender()
{
	const auto &model = getModelMatrix();
	shader.bind();
	scene.entityPreRender(*this);
	shader.setBlock("Model", model);
	shader.setBlock("View", scene.view.matrix);
	shader.setBlock("Projection", scene.projection.matrix);
	shader.setBlock("CameraPosition", scene.view.position, 16);
	shader.unbind();
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
		{ 0, -actualNewSize.y, 0 }, { actualNewSize.x, -actualNewSize.y, 0 }, { actualNewSize.x, 0, 0 }, { 0, 0, 0 }
	};
	updateElements("Position", positions);
	size = actualNewSize;
}