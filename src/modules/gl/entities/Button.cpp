#include <anex/modules/gl/entities/Button.hpp>
#include <anex/utilities.hpp>
#include <iostream>
using namespace anex::modules::gl::entities;
Button::Button(GLWindow &window,
										 GLScene &scene,
										 glm::vec3 position,
										 glm::vec3 rotation,
										 glm::vec3 scale,
										 glm::vec4 color,
              			 glm::vec2  size,
										 const std::string_view text,
										 fonts::freetype::FreetypeFont &font,
               			 const OnClickHandler &handler,
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
    name.empty() ? "Button " + std::to_string(++buttonsCount) : name
	),
	scene(scene),
	size({ size.x / window.windowWidth / 0.5, size.y / window.windowHeight / 0.5 }),
	text(text),
	font(font),
	handler(handler)
{
	updateIndices(indices);
  colors.resize(4);
	setColor(color);
	float FontSize = size.y / 1.3f;
	float LineHeight = 0;
	auto TextSize = font.stringSize(text, FontSize, LineHeight, {0, 0});
	TextSize.y /= window.windowHeight * 0.5f;
	TextSize.x /= window.windowWidth * 0.5f;
	textView = std::make_shared<TextView>(
		window,
		scene,
		glm::vec3(this->size.x / 2, -this->size.y / 2, 0.5f),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec4(1, 1, 1, 1),
		text,
		TextSize,
		font,
		FontSize,
		true,
		TextView::RepositionHandler(),
		TextView::ResizeHandler(),
		[&]
		{
			auto &glWindow = ((VAO&)*this).window;
			return this->size.y * window.windowHeight * 0.5f / 1.2f;
		});
	textView->addToBVH = false;
  addChild(textView);
  setSize(this->size);
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
		if (pressed)
			if (this->handler)
        this->handler();
	});
};
Button::~Button()
{
	removeMouseHoverHandler(mouseHoverID);
	removeMousePressHandler(0, mousePressID);
};
void Button::preRender()
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
void Button::setColor(glm::vec4 color)
{
  auto colorsData = colors.data();
	colorsData[0] = color;
  colorsData[1] = color;
  colorsData[2] = color;
  colorsData[3] = color;
	updateElements("Color", colors);
};
void Button::setSize(glm::vec2 newSize)
{
	positions = {
		{ 0, -newSize.y, 0 }, { newSize.x, -newSize.y, 0 }, { newSize.x, 0, 0 }, { 0, 0, 0 }
	};
	updateElements("Position", positions);
	this->size = newSize;
};