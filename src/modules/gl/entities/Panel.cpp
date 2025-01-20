#include <anex/modules/gl/entities/Panel.hpp>
#include <anex/utilities.hpp>
#include <iostream>
using namespace anex::modules::gl::entities;
PanelMenu::PanelMenu(anex::modules::gl::GLWindow &window,
				             anex::modules::gl::GLScene &scene,
				             glm::vec3 position,
				             glm::vec3 rotation,
				             glm::vec3 scale,
				             glm::vec4 color,
				             fonts::freetype::FreetypeFont &font,
										 const std::string_view title,
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
		name.empty() ? "PanelMenu " + std::to_string(++panelMenusCount) : name
	),
	scene(scene),
	size({ 0, 0 }),
  color(color),
	font(font),
	title(title),
	width(width),
	height(height)
{
	updateIndices(indices);
	setColor(color);
	updateElements("Position", positions);
  float titleLineHeight = 0;
	auto titleSize = font.stringSize(title, window.windowHeight / 30.f, titleLineHeight, glm::vec2(0));
	titleSize.y /= window.windowHeight * 0.5f;
	titleSize.x /= window.windowWidth * 0.5f;
	titleTextView = std::make_shared<TextView>(
		window,
		scene,
		glm::vec3(titleSize.x / 2, -titleSize.y / 2, 0.1),
		glm::vec3(0),
		glm::vec3(1),
		title,
		titleSize,
		font,
		window.windowHeight / 30.f,
		true,
		[](auto titleSize)
		{
			return glm::vec3(titleSize.x / 2, -titleSize.y / 2, 0.1);
		});
  titleTextView->addToBVH = false;
  addChild(titleTextView);
  setSize();
  addToBVH = false;
};
void PanelMenu::addItem(std::string_view name, GLEntity &entity)
{
	auto &vao = (VAO &)*this;
	float sizeYTotal = 0;
	for (auto &child : children)
	{
    auto textViewPointer = std::dynamic_pointer_cast<TextView>(child.second);
    if (textViewPointer)
    {
      sizeYTotal += textViewPointer->size.y;
    	continue;
    }
		auto &childItem = *std::dynamic_pointer_cast<PanelItem>(child.second);
		sizeYTotal += childItem.size.y;
	}
  static const auto indent = 16.f;
	auto &glWindow = ((VAO &)*this).window;
  auto panelItem = std::make_shared<PanelItem>(
  	vao.window,
		scene,
		glm::vec3(indent / glWindow.windowWidth / 0.5, -sizeYTotal, 0.1),
		glm::vec3(0),
		glm::vec3(1),
    color,
		name,
		font,
		entity,
    width,
    indent);
  addChild(panelItem);
  scene.postAddEntity(panelItem, {ID, panelItem->ID});
  sizeYTotal += panelItem->size.y;
	float sizeXMax = 0;
	for (auto &child : children)
	{
		auto textViewPointer = std::dynamic_pointer_cast<TextView>(child.second);
		if (textViewPointer)
		{
			sizeXMax = std::max(textViewPointer->size.x, sizeXMax);
			continue;
		}
		auto &childItem = *std::dynamic_pointer_cast<PanelItem>(child.second);
		sizeXMax = std::max(childItem.size.x, sizeXMax);
	}
	for (auto &child : children)
	{
		auto childItemPointer = std::dynamic_pointer_cast<PanelItem>(child.second);
    if (!childItemPointer)
      continue;
    auto &childItem = *childItemPointer;
		childItem.setSize({sizeXMax, childItem.size.y});
    scene.bvh.updateEntity(childItem);
	}
	setSize();
};
void PanelMenu::preRender()
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
void PanelMenu::setColor(glm::vec4 color)
{
	colors = {color, color, color, color};
	updateElements("Color", colors);
};
void PanelMenu::setSize()
{
	auto &glWindow = ((VAO &)*this).window;
	glm::vec2 newSize(width / glWindow.windowWidth / 0.5, height / glWindow.windowHeight / 0.5);
	positions = {
		{ 0, -newSize.y, 0 }, { newSize.x, -newSize.y, 0 }, { newSize.x, 0, 0 }, { 0, 0, 0 }
	};
	updateElements("Position", positions);
	this->size = newSize;
};
PanelItem::PanelItem(GLWindow &window,
										 GLScene &scene,
										 glm::vec3 position,
										 glm::vec3 rotation,
										 glm::vec3 scale,
										 glm::vec4 color,
										 const std::string_view text,
										 fonts::freetype::FreetypeFont &font,
                     GLEntity &entity,
                     float panelWidth,
                     float indent,
										 const shaders::RuntimeConstants &constants,
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
    name.empty() ? "PanelItem " + std::to_string(++panelItemsCount) : name
	),
	scene(scene),
	size({ 0, 0 }),
	text(text),
	font(font),
	entity(entity),
  panelWidth(panelWidth),
  indent(indent)
{
	updateIndices(indices);
  colors.resize(4);
	setColor(color);
	float FontSize = window.windowHeight / 30.f;
	float LineHeight = 0;
	auto TextSize = font.stringSize(text, FontSize, LineHeight, {panelWidth - indent, 0});
	TextSize.y /= window.windowHeight * 0.5f;
	TextSize.x /= window.windowWidth * 0.5f;
	textView = std::make_shared<TextView>(
		window,
		scene,
		glm::vec3(TextSize.x / 2, -TextSize.y / 2, 0.1f),
		glm::vec3(0),
		glm::vec3(1),
		text,
		TextSize,
		font,
		FontSize,
		true,
		[](auto TextSize)
		{
			return glm::vec3(TextSize.x / 2, -TextSize.y / 2, 0.1f);
		});
	textView->addToBVH = false;
  addChild(textView);
  setSize({ (panelWidth - indent) / (window.windowWidth / 2), TextSize.y });
	mouseHoverID = addMouseHoverHandler([&, color](const auto &hovered)
	{
		if (hovered)
		{
			setColor({ 0.4, 0.4, 0.4, 1 });
		}
		else
		{
			setColor(color);
		}
	});
	mousePressID = addMousePressHandler(0, [&](auto pressed)
	{
		if (!pressed)
			return;
	});
};
PanelItem::~PanelItem()
{
	removeMouseHoverHandler(mouseHoverID);
	removeMousePressHandler(0, mousePressID);
};
void PanelItem::preRender()
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
void PanelItem::setColor(glm::vec4 color)
{
  auto colorsData = colors.data();
	colorsData[0] = color;
  colorsData[1] = color;
  colorsData[2] = color;
  colorsData[3] = color;
	updateElements("Color", colors);
};
void PanelItem::setSize(glm::vec2 newSize)
{
	positions = {
		{ 0, -newSize.y, 0 }, { newSize.x, -newSize.y, 0 }, { newSize.x, 0, 0 }, { 0, 0, 0 }
	};
	updateElements("Position", positions);
	this->size = newSize;
};