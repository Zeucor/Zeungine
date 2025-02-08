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
				   std::string_view name) : Entity(window,
												   {{"UV2", "Position", "Normal", "Texture2D",
													 "View", "Projection", "Model", "CameraPosition",
													 "TextColor"}},
												   6,
												   {
													   0, 1, 2, 2, 3, 0 // Front face
												   },
												   4,
												   {
													   {-size.x / 2, -size.y / 2, 0}, {size.x / 2, -size.y / 2, 0}, {size.x / 2, size.y / 2, 0}, {-size.x / 2, size.y / 2, 0} // Front
												   },
												   position,
												   rotation,
												   scale,
												   name.empty() ? "TextView " + std::to_string(++textViewsCount) : name),
											uvs({
												// Front face
												{0, 0}, // 0
												{1, 0}, // 1
												{1, 1}, // 2
												{0, 1}	// 3
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
	switch (window.iRenderer->renderer)
	{
	default:
		break;
	case RENDERER_VULKAN:
	case RENDERER_METAL:
		flipUVsY(uvs);
		break;
	}
	computeNormals(indices, positions, normals);
	updateIndices(indices);
	updateElements("UV2", uvs);
	updateElements("Position", positions);
	updateElements("Normal", normals);
	resizeID = window.addResizeHandler([&](auto newSize)
									   { forceUpdate(); });
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
#ifdef USE_VULKAN
		fontSize *= 2.0;
#endif
	}
	float lineHeight = 0;
	auto TextSize = textSize = font.stringSize(text, fontSize, lineHeight, {0, 0});
	if (TextSize.x && TextSize.y)
		font.stringToTexture(text, {1, 1, 1, 1}, fontSize, lineHeight, TextSize, texturePointer, cursorIndex, cursorPosition);
	actualSizeBeforeNDC = TextSize;
	if (textSizeIsNDC)
	{
#ifdef USE_VULKAN
		TextSize.x /= this->window.windowWidth;
		TextSize.y /= this->window.windowHeight;
#else
		TextSize.x /= this->window.windowWidth * 0.5;
		TextSize.y /= this->window.windowHeight * 0.5;
#endif
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
bool TextView::preRender()
{
	if (!size.x || !size.y)
		return false;
	auto &model = getModelMatrix();
	shader->bind(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, scene.view.matrix);
	shader->setBlock("Projection", *this, scene.projection.matrix);
	shader->setBlock("CameraPosition", *this, scene.view.position, 16);
	shader->setTexture("Texture2D", *this, *texturePointer, 0);
	shader->setUniform("TextColor", *this, textColor);
	shader->unbind();
	return true;
}
void TextView::setSize(glm::vec2 size)
{
	this->size = size;
	positions = {
		{-size.x / 2, -size.y / 2, 0}, {size.x / 2, -size.y / 2, 0}, {size.x / 2, size.y / 2, 0}, {-size.x / 2, size.y / 2, 0} // Front
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