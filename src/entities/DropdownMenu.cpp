#include <zg/entities/DropdownMenu.hpp>
#include <zg/utilities.hpp>
using namespace zg::entities;
DropdownMenu::DropdownMenu(zg::Window &window,
							             zg::Scene &scene,
							             glm::vec3 position,
							             glm::vec3 rotation,
							             glm::vec3 scale,
							             glm::vec4 color,
                           fonts::freetype::FreetypeFont &font,
							             const zg::shaders::RuntimeConstants &constants,
							             std::string_view name):
	zg::Entity(
		window,
		zg::mergeVectors<std::string_view>({
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
		name.empty() ? "DropdownMenu " + std::to_string(++dropdownMenusCount) : name
	),
	scene(scene),
	size({ 0, 0 }),
	font(font)
{
	updateIndices(indices);
	setColor(color);
	updateElements("Position", positions);
};
void DropdownMenu::addOption(std::string_view name, const OptionPressHandler &handler)
{
	auto &vao = (VAO &)*this;
	float sizeYTotal = 0;
	for (auto &child : children)
	{
		auto &childItem = *std::dynamic_pointer_cast<DropdownItem>(child.second);
		sizeYTotal += childItem.size.y;
	}
  auto dropdownItem = std::make_shared<DropdownItem>(dynamic_cast<Window&>(window), scene, glm::vec3(0, -sizeYTotal, 0.5), glm::vec3(0), glm::vec3(1), glm::vec4(0.1, 0.1, 0.1, 1), name, handler, font);
  addChild(dropdownItem);
  sizeYTotal += dropdownItem->size.y;
	float sizeXMax = 0;
	for (auto &child : children)
	{
		auto &childItem = *std::dynamic_pointer_cast<DropdownItem>(child.second);
		sizeXMax = std::max(childItem.size.x, sizeXMax);
	}
	for (auto &child : children)
	{
		auto &childItem = *std::dynamic_pointer_cast<DropdownItem>(child.second);
		childItem.setSize({sizeXMax, childItem.size.y});
	}
	setSize({sizeXMax, sizeYTotal});
};
void DropdownMenu::preRender()
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
void DropdownMenu::setColor(glm::vec4 color)
{
	colors = {color, color, color, color};
	updateElements("Color", colors);
};
void DropdownMenu::setSize(glm::vec2 size)
{
	positions = {
		{ 0, -size.y, 0 }, { size.x, -size.y, 0 }, { size.x, 0, 0 }, { 0, 0, 0 }
	};
	updateElements("Position", positions);
	this->size = size;
};
DropdownItem::DropdownItem(Window &window,
													 Scene &scene,
													 glm::vec3 position,
													 glm::vec3 rotation,
													 glm::vec3 scale,
													 glm::vec4 color,
													 const std::string_view text,
													 const DropdownMenu::OptionPressHandler &handler,
													 fonts::freetype::FreetypeFont &font,
													 const shaders::RuntimeConstants &constants,
													 std::string_view name):
	zg::Entity(
		window,
		zg::mergeVectors<std::string_view>({
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
		name.empty() ? "DropdownItem " + std::to_string(++dropdownItemsCount) : name
	),
	scene(scene),
	size({ 0, 0 }),
	text(text),
	handler(handler),
	font(font)
{
	updateIndices(indices);
	setColor(color);
	float FontSize = window.windowHeight / 32.f;
	float LineHeight = 0;
	auto TextSize = font.stringSize(text, FontSize, LineHeight, {0, 0});
	TextSize.x /= window.windowWidth * 0.5;
	TextSize.y /= window.windowHeight * 0.5;
	textView = std::make_shared<TextView>(
		window,
		scene,
		glm::vec3(TextSize.x / 2, -TextSize.y / 2, 0.1f),
		glm::vec3(0),
		glm::vec3(1, 1, 1),
		glm::vec4(1, 1, 1, 1),
		text,
		TextSize,
		font,
		FontSize,
		true,
		[](auto TextSize)
		{
			return glm::vec3(TextSize.x / 2, -TextSize.y / 2, 0.1f);
		},
		TextView::ResizeHandler(),
		[&]
		{
			return this->window.windowHeight / 32.f;
		});
	textView->addToBVH = false;
  addChild(textView);
  setSize({ TextSize.x, TextSize.y });
	mouseHoverID = addMouseHoverHandler([&, color](const auto &hovered)
	{
		if (hovered)
		{
			setColor({ 0.5, 0.5, 0.5, 1 });
		}
		else
		{
			setColor(color);
		}
	});
	mousePressID = addMousePressHandler(0, [&](auto pressed)
	{
		if (!pressed)
			this->handler();
	});
};
DropdownItem::~DropdownItem()
{
	removeMouseHoverHandler(mouseHoverID);
	removeMousePressHandler(0, mousePressID);
};
void DropdownItem::preRender()
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
void DropdownItem::setColor(glm::vec4 color)
{
	colors = {color, color, color, color};
	updateElements("Color", colors);
};
void DropdownItem::setSize(glm::vec2 size)
{
	positions = {
		{ 0, -size.y, 0 }, { size.x, -size.y, 0 }, { size.x, 0, 0 }, { 0, 0, 0 }
	};
	updateElements("Position", positions);
	this->size = size;
};