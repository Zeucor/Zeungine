#include <zg/entities/TextView.hpp>
#include <zg/utilities.hpp>
using namespace zg::entities;
TextView::TextView(Window &window,
                   Scene &scene,
                   glm::vec3 position,
                   glm::vec3 rotation,
                   glm::vec3 scale,
                   glm::vec4 textColor,
                   const std::string_view text,
                   glm::vec2 size,
                   fonts::freetype::FreetypeFont &font,
                   float fontSize,
                   bool textSizeIsNDC,
                   const RepositionHandler &repositionHandler,
                   const ResizeHandler &resizeHandler,
                   const ReFontSizeHandler &reFontSizeHandler,
                   std::string_view name):
	Entity(window,
		{
			{
				"UV2", "Position", "Normal", "Texture2D",
				"View", "Projection", "Model", "CameraPosition",
				"TextColor"
			}
		},
		6,
		{
			0, 1, 2,  2, 3, 0   // Front face
		},
		4,
		{
			{ -size.x / 2, -size. y / 2, 0}, { size.x / 2, -size. y / 2, 0}, { size.x / 2,  size. y / 2, 0}, { -size.x / 2,  size. y / 2, 0} // Frontm
		},
		position,
		rotation,
		scale,
		name.empty() ? "TextView " + std::to_string(++textViewsCount) : name
  ),
	uvs({
		// Front face
		{ 0, 0 },  // 0
		{ 1, 0 },  // 1
		{ 1, 1 },  // 2
		{ 0, 1 }  // 3
	}),
	scene(scene),
	textColor(textColor),
	text(text),
	size(size),
	font(font),
	fontSize(fontSize),
	textSizeIsNDC(textSizeIsNDC),
	repositionHandler(repositionHandler),
	resizeHandler(resizeHandler),
	reFontSizeHandler(reFontSizeHandler)
{
	computeNormals(indices, positions, normals);
	updateIndices(indices);
	updateElements("UV2", uvs);
	updateElements("Position", positions);
	updateElements("Normal", normals);
	TextView::update();
	resizeID = window.addResizeHandler([&](auto newSize)
	{
		forceUpdate();
	});
	setTextColor(textColor);
}
TextView::~TextView()
{
	window.removeResizeHandler(resizeID);
}
void TextView::update()
{
	if (oldText != text)
	{
		forceUpdate();
	}
}
void TextView::forceUpdate()
{
	if (reFontSizeHandler)
	{
		fontSize = reFontSizeHandler();
	}
	float lineHeight = 0;
	auto TextSize = textSize = font.stringSize(text, fontSize, lineHeight, {0, 0});
	if (TextSize.x && TextSize.y)
		font.stringToTexture(text, {1, 1, 1, 1}, fontSize, lineHeight, TextSize, texturePointer, cursorIndex, cursorPosition);
	actualSizeBeforeNDC = TextSize;
	if (textSizeIsNDC)
	{
		TextSize.x /= this->window.windowWidth * 0.5;
		TextSize.y /= this->window.windowHeight * 0.5;
	}
	actualSize = TextSize;
	if (resizeHandler)
	{
		setSize(resizeHandler(TextSize));
	}
	else
	{
		setSize(TextSize);
	}
	if (repositionHandler)
	{
		position = repositionHandler(this->size);
	}
	oldText = text;
}
void TextView::preRender()
{
  auto &model = getModelMatrix();
	shader.bind();
	shader.setBlock("Model", model);
	shader.setBlock("View", scene.view.matrix);
	shader.setBlock("Projection", scene.projection.matrix);
	shader.setBlock("CameraPosition", scene.view.position, 16);
	if (size.x && size.y)
	  shader.setTexture("Texture2D", *texturePointer, 0);
	shader.setUniform("TextColor", textColor);
	shader.unbind();
}
void TextView::setSize(glm::vec2 size)
{
	this->size = size;
	positions = {
		{ -size.x / 2, -size. y / 2, 0}, { size.x / 2, -size. y / 2, 0}, { size.x / 2,  size. y / 2, 0}, { -size.x / 2,  size. y / 2, 0} // Frontm
	};
	updateElements("Position", positions);
}
void TextView::updateText(const std::string_view text)
{
	this->text = text;
}
void TextView::setTextColor(glm::vec4 newTextColor)
{
	textColor = newTextColor;
}
void TextView::forceReposition()
{
	if (repositionHandler)
		position = repositionHandler(this->size);
}