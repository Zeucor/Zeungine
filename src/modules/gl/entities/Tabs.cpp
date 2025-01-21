#include <anex/modules/gl/entities/Tabs.hpp>
#include <anex/utilities.hpp>
#include <iostream>
using namespace anex::modules::gl::entities;
TabsBar::TabsBar(anex::modules::gl::GLWindow &window,
				             anex::modules::gl::GLScene &scene,
				             glm::vec3 position,
				             glm::vec3 rotation,
				             glm::vec3 scale,
				             glm::vec4 color,
										 fonts::freetype::FreetypeFont& font,
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
		name.empty() ? "TabsBar " + std::to_string(++tabBarsCount) : name
	),
	scene(scene),
	size({ 0, 0 }),
  color(color),
	font(font),
	width(width),
	height(height)
{
	updateIndices(indices);
	setColor(color);
  setSize();
  addToBVH = false;
};
void TabsBar::addTab(std::string_view name, const TabClickHandler &handler, bool active)
{
	auto &vao = (VAO &)*this;
	float sizeXTotal = 0;
	for (auto &child : children)
	{
		auto &childItem = *std::dynamic_pointer_cast<Tab>(child.second);
		sizeXTotal += childItem.size.x;
	}
  static const auto indent = 16.f;
  auto panelItem = std::make_shared<Tab>(
  	vao.window,
		scene,
		glm::vec3(sizeXTotal, 0, 0.1),
		glm::vec3(0),
		glm::vec3(1),
    color,
		name,
		font,
    height,
    handler,
    active,
    *this);
  addChild(panelItem);
  scene.postAddEntity(panelItem, {ID, panelItem->ID});
	setSize();
};
void TabsBar::preRender()
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
void TabsBar::setColor(glm::vec4 color)
{
	colors = {color, color, color, color};
	updateElements("Color", colors);
};
void TabsBar::setSize()
{
	auto &glWindow = ((VAO &)*this).window;
	glm::vec2 newSize(width / glWindow.windowWidth / 0.5, height / glWindow.windowHeight / 0.5);
	positions = {
		{ 0, -newSize.y, 0 }, { newSize.x, -newSize.y, 0 }, { newSize.x, 0, 0 }, { 0, 0, 0 }
	};
	updateElements("Position", positions);
	this->size = newSize;
};
void TabsBar::markInactive(Tab *activeTab)
{
  for (auto &child : children)
  {
    auto &childItem = *std::dynamic_pointer_cast<Tab>(child.second);
    if (&childItem != activeTab && childItem.active)
    {
      childItem.markInactive();
    }
  }
};
Tab::Tab(GLWindow &window,
										 GLScene &scene,
										 glm::vec3 position,
										 glm::vec3 rotation,
										 glm::vec3 scale,
										 glm::vec4 color,
										 const std::string_view text,
										 fonts::freetype::FreetypeFont &font,
										 float height,
										 const TabsBar::TabClickHandler &handler,
										 bool active,
										 TabsBar &tabsBar,
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
    name.empty() ? "Tab " + std::to_string(++tabsCount) : name
	),
	scene(scene),
	size({ 0, 0 }),
	text(text),
	font(font),
	height(height),
	NDCHeight((height / window.windowHeight) * 2),
  handler(handler),
	active(active),
	activeColor(std::clamp(color[0] * 2.f, 0.f, 1.f), std::clamp(color[1] * 2.f, 0.f, 1.f), std::clamp(color[2] * 2.f, 0.f, 1.f), color[3]),
  inactiveColor(color),
  tabsBar(tabsBar)
{
	updateIndices(indices);
  colors.resize(4);
	setColor(active ? activeColor : inactiveColor);
	float FontSize = height / 1.5f;
	float LineHeight = 0;
	auto TextSize = font.stringSize(text, FontSize, LineHeight, {0, 0});
	TextSize.y /= window.windowHeight * 0.5f;
	TextSize.x /= window.windowWidth * 0.5f;
	textView = std::make_shared<TextView>(
		window,
		scene,
		glm::vec3(TextSize.x / 2 + 8 / window.windowWidth, -NDCHeight / 2, 0.1f),
		glm::vec3(0),
		glm::vec3(1),
		text,
		TextSize,
		font,
		FontSize,
		true,
		TextView::RepositionHandler(),
		TextView::RepositionHandler(),
		[&]
		{
			auto &glWindow = ((VAO&)*this).window;
			return NDCHeight * glWindow.windowHeight * 0.5f / 1.5f;
		});
	textView->addToBVH = false;
  addChild(textView);
  setSize({TextSize.x + 16 / window.windowWidth, height / window.windowHeight / 0.5});
	mouseHoverID = addMouseHoverHandler([&, color](const auto &hovered)
	{
		this->hovered = hovered;
		if (hovered)
		{
			setColor(this->active ? this->activeColor : glm::vec4( 0.4, 0.4, 0.4, 1 ));
		}
		else
		{
			setColor(this->active ? this->activeColor : inactiveColor);
		}
	});
	mousePressID = addMousePressHandler(0, [&](auto pressed)mutable
	{
		if (!pressed && !this->active)
  	{
			this->active = true;
			this->handler();
      tabsBar.markInactive(this);
		}
	});
};
Tab::~Tab()
{
	removeMouseHoverHandler(mouseHoverID);
	removeMousePressHandler(0, mousePressID);
};
void Tab::preRender()
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
void Tab::setColor(glm::vec4 color)
{
  auto colorsData = colors.data();
	colorsData[0] = color;
  colorsData[1] = color;
  colorsData[2] = color;
  colorsData[3] = color;
	updateElements("Color", colors);
};
void Tab::setSize(glm::vec2 newSize)
{
	positions = {
		{ 0, -newSize.y, 0 }, { newSize.x, -newSize.y, 0 }, { newSize.x, 0, 0 }, { 0, 0, 0 }
	};
	updateElements("Position", positions);
	this->size = newSize;
};
void Tab::markInactive()
{
  active = false;
  setColor(inactiveColor);
};