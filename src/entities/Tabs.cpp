#include <iostream>
#include <zg/entities/Tabs.hpp>
#include <zg/utilities.hpp>
#include <zg/images/SVGRasterize.hpp>
using namespace zg::entities;
TabsBar::TabsBar(zg::Window& window, zg::Scene& scene, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
								 glm::vec4 color, fonts::freetype::FreetypeFont& font, float width, float height,
								 const zg::shaders::RuntimeConstants& constants, std::string_view name) :
		zg::Entity(window,
							 zg::mergeVectors<std::string_view>(
								 {{"Color", "Position", "View", "Projection", "Model", "CameraPosition"}}, constants),
							 6, {0, 1, 2, 2, 3, 0}, 4, {{0, -0, 0}, {0, -0, 0}, {0, 0, 0}, {0, 0, 0}}, position, rotation, scale,
							 name.empty() ? "TabsBar " + std::to_string(++tabBarsCount) : name),
		scene(scene), size({0, 0}), color(color), font(font), width(width), height(height)
{
	updateIndices(indices);
	setColor(color);
	setSize();
	addToBVH = false;
};
size_t TabsBar::addTab(std::string_view name, const TabClickHandler& handler, bool active, const zgfilesystem::File& iconFile)
{
	auto& vao = (VAO&)*this;
	float sizeXTotal = 0;
	for (auto& child : children)
	{
		auto& childItem = *std::dynamic_pointer_cast<Tab>(child.second);
		sizeXTotal += childItem.size.x;
	}
	static const auto indent = 16.f;
	auto panelItem = std::make_shared<Tab>(window, scene, glm::vec3(sizeXTotal, 0, 0.1), glm::vec3(0), glm::vec3(1),
																				 color, name, font, height, handler, active, *this, iconFile);
	addChild(panelItem);
	scene.postAddEntity(panelItem, {ID, panelItem->ID});
	setSize();
	return panelItem->ID;
};
void TabsBar::removeTab(size_t ID) { removeChild(ID); }
bool TabsBar::preRender()
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
};
void TabsBar::setColor(glm::vec4 color)
{
	colors = {color, color, color, color};
	updateElements("Color", colors);
};
void TabsBar::setSize()
{
	glm::vec2 newSize(width / window.windowWidth / 0.5, height / window.windowHeight / 0.5);
	positions = {{0, -newSize.y, 0}, {newSize.x, -newSize.y, 0}, {newSize.x, 0, 0}, {0, 0, 0}};
	updateElements("Position", positions);
	this->size = newSize;
};
void TabsBar::markInactive(Tab* activeTab)
{
	for (auto& child : children)
	{
		auto& childItem = *std::dynamic_pointer_cast<Tab>(child.second);
		if (&childItem != activeTab && childItem.active)
		{
			childItem.markInactive();
		}
	}
};
Tab::Tab(Window& window, Scene& scene, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, glm::vec4 color,
				 const std::string_view text, fonts::freetype::FreetypeFont& font, float height,
				 const TabsBar::TabClickHandler& handler, bool active, TabsBar& tabsBar, const zgfilesystem::File& iconFile,
				 const shaders::RuntimeConstants& constants, std::string_view name) :
		zg::Entity(window,
							 zg::mergeVectors<std::string_view>(
								 {{"Color", "Position", "View", "Projection", "Model", "CameraPosition"}}, constants),
							 6, {0, 1, 2, 2, 3, 0}, 4, {{0, -0, 0}, {0, -0, 0}, {0, 0, 0}, {0, 0, 0}}, position, rotation, scale,
							 name.empty() ? "Tab " + std::to_string(++tabsCount) : name),
		scene(scene), size({0, 0}), text(text), font(font), height(height), NDCHeight((height / window.windowHeight) * 2),
		handler(handler), active(active),
		activeColor(std::clamp(color[0] * 2.f, 0.f, 1.f), std::clamp(color[1] * 2.f, 0.f, 1.f),
								std::clamp(color[2] * 2.f, 0.f, 1.f), color[3]),
		inactiveColor(color), tabsBar(tabsBar)
{
	updateIndices(indices);
	colors.resize(4);
	setColor(active ? activeColor : inactiveColor);
	float fontSize = height / 1.5f;
	float iconTextureScale = 4;
	float iconSize = height;
	float iconTextureSize = height * iconTextureScale;
	static float padding = 64;
	auto iconBitmap = images::SVGRasterize(iconFile, glm::ivec2(iconTextureSize, iconTextureSize));
	iconTexture = std::make_shared<textures::Texture>(
		window,
		glm::ivec4(iconTextureSize, iconTextureSize, 1, 1),
		(void*)iconBitmap.get(),
		textures::Texture::Format::RGBA8,
		textures::Texture::Type::UnsignedByte,
		textures::Texture::FilterType::Linear
	);
	iconPlane = std::make_shared<entities::Plane>(
		window,
		scene,
		glm::vec3(iconSize / window.windowWidth / 2 + (padding / 4) / window.windowWidth / 2, -NDCHeight / 2, 0.1),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec2(iconSize / window.windowWidth, iconSize / window.windowHeight),
		*iconTexture,
		shaders::RuntimeConstants()
	);
	addChild(iconPlane);
	float LineHeight = 0;
	auto TextSize = font.stringSize(text, fontSize, LineHeight, {0, 0});
	TextSize.y /= window.windowHeight * 0.5f;
	TextSize.x /= window.windowWidth * 0.5f;
	textView = std::make_shared<TextView>(
		window, scene, glm::vec3(TextSize.x / 2 + (padding / 2) / window.windowWidth / 2 + iconPlane->size.x, -NDCHeight / 2, 0.1f), glm::vec3(0), glm::vec3(1),
		glm::vec4(1, 1, 1, 1), text, TextSize, font, fontSize, true, TextView::RepositionHandler(),
		TextView::RepositionHandler(), [&] { return NDCHeight * this->window.windowHeight * 0.5f / 1.5f; });
	textView->addToBVH = false;
	addChild(textView);
	setSize({TextSize.x + padding / window.windowWidth / 2 + iconPlane->size.x, height / window.windowHeight / 0.5});
	mouseHoverID = addMouseHoverHandler(
		[&, color](const auto& hovered)
		{
			this->hovered = hovered;
			if (hovered)
			{
				setColor(this->active ? this->activeColor : glm::vec4(0.4, 0.4, 0.4, 1));
			}
			else
			{
				setColor(this->active ? this->activeColor : inactiveColor);
			}
		});
	mousePressID = addMousePressHandler(0,
																			[&](auto pressed) mutable
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
bool Tab::preRender()
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
	positions = {{0, -newSize.y, 0}, {newSize.x, -newSize.y, 0}, {newSize.x, 0, 0}, {0, 0, 0}};
	updateElements("Position", positions);
	this->size = newSize;
};
void Tab::markInactive()
{
	active = false;
	setColor(inactiveColor);
};
