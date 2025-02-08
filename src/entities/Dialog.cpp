#include <zg/entities/Dialog.hpp>
#include <zg/utilities.hpp>
#include <iostream>
using namespace zg::entities;
Dialog::Dialog(zg::Window &window,
			   zg::Scene &scene,
			   glm::vec3 position,
			   glm::vec3 rotation,
			   glm::vec3 scale,
			   glm::vec4 color,
			   fonts::freetype::FreetypeFont &font,
			   const std::string_view title,
			   float width,
			   float height,
			   const std::vector<std::shared_ptr<Entity>> &children,
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
												   name.empty() ? "Dialog " + std::to_string(++dialogsCount) : name),
										scene(scene),
										size({0, 0}),
										color(color),
										font(font),
										title(title),
										width(width),
										height(height),
										NDCHeight((height / window.windowHeight) * 2.f),
										fontSize(height / 5.5f)
{
	updateIndices(indices);
	setColor(color);
	updateElements("Position", positions);
	float titleLineHeight = 0;
	auto titleSize = font.stringSize(title, 20.f, titleLineHeight, glm::vec2(0));
	titleSize.y /= window.windowHeight * 0.5f;
	titleSize.x /= window.windowWidth * 0.5f;
	glm::vec2 dialogSize(width / window.windowWidth / 0.5, height / window.windowHeight / 0.5);
	glm::vec3 titlePosition(titleSize.x / 2, -titleSize.y / 2, 0.1);
	titlePosition.x -= dialogSize.x / 2;
	titlePosition.y += dialogSize.y / 2;
	titleTextView = std::make_shared<TextView>(
		window,
		scene,
		titlePosition,
		glm::vec3(0),
		glm::vec3(1),
		glm::vec4(1, 1, 1, 1),
		title,
		titleSize,
		font,
		fontSize,
		true,
		[dialogSize](auto titleSize)
		{
			return glm::vec3(titleSize.x / 2 - dialogSize.x / 2, -titleSize.y / 2 + dialogSize.y / 2, 0.1);
		},
		TextView::ResizeHandler(),
		[&]
		{
			return (this->fontSize = ((this->NDCHeight * this->window.windowHeight / 2.f) / 5.5f));
		});
	titleTextView->addToBVH = false;
	addChild(titleTextView);
	for (const auto &child : children)
	{
		child->position.x -= dialogSize.x / 2;
		child->position.y += dialogSize.y / 2;
		addChild(child);
	}
	setSize(dialogSize);
};
void Dialog::preRender()
{
	const auto &model = getModelMatrix();
	shader.bind(*this);
	scene.entityPreRender(*this);
	shader.setBlock("Model", *this, model);
	shader.setBlock("View", *this, scene.view.matrix);
	shader.setBlock("Projection", *this, scene.projection.matrix);
	shader.setBlock("CameraPosition", *this, scene.view.position, 16);
	shader.unbind();
};
void Dialog::setColor(glm::vec4 color)
{
	colors = {color, color, color, color};
	updateElements("Color", colors);
};
void Dialog::setSize(glm::vec2 newSize)
{
	positions = {
		{-newSize.x / 2, -newSize.y / 2, 0}, {newSize.x / 2, -newSize.y / 2, 0}, {newSize.x / 2, newSize.y / 2, 0}, {-newSize.x / 2, newSize.y / 2, 0}};
	updateElements("Position", positions);
	this->size = newSize;
};