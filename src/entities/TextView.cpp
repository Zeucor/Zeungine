#include <zg/entities/TextView.hpp>
#include <zg/utilities.hpp>
using namespace zg::entities;
TextView::TextView(Window &window,
				   Scene &scene,
				   glm::vec3 position,
				   glm::quat rotation,
				   glm::vec3 scale,
				   glm::vec4 textColor,
				   const std::string_view text,
				   glm::vec2 size,
				   fonts::freetype::FreetypeFont &font,
				   float fontSize,
				   bool textSizeIsNDC,
				   glm::vec2 bounds,
				   enums::EBreakStyle breakStyle,
				   const RepositionHandler &repositionHandler,
				   const ResizeHandler &resizeHandler,
				   const ReFontSizeHandler &reFontSizeHandler,
				   std::string_view name) : Entity(window, scene,
												   {{"Position", "Normal",
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
											
											textColor(textColor),
											text(text),
											font(font),
											fontSize(fontSize),
											textSizeIsNDC(textSizeIsNDC),
											bounds(bounds),
											breakStyle(breakStyle),
											repositionHandler(repositionHandler),
											resizeHandler(resizeHandler),
											reFontSizeHandler(reFontSizeHandler)
{
	computeNormals(indices, positions, normals);
	updateIndices(indices);
	// updateElements("UV2", uvs);
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
void TextView::preUpdate()
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
// #ifdef USE_VULKAN
		fontSize *= 2.0;
// #endif
	}
	float lineHeight = 0;
	auto TextSize = textSize = font.stringSize(text, fontSize, lineHeight, bounds, breakStyle);
	if (TextSize.x && TextSize.y)
		font.stringToEntity(text, {0, 0, 0}, {1, 1, 1, 1}, fontSize, lineHeight, TextSize, breakStyle, scene, *this, existingAndUpdatedGlyphs, cursorIndex, cursor);
	actualSizeBeforeNDC = TextSize;
	if (textSizeIsNDC)
	{
// #ifdef USE_VULKAN
		TextSize.x /= this->window.windowWidth * 0.5;
		TextSize.y /= this->window.windowHeight;
// #else
// 		TextSize.x /= this->window.windowWidth * 0.5;
// 		TextSize.y /= this->window.windowHeight * 0.5;
// #endif
	}
	actualSize = TextSize;
	if (resizeHandler)
	{
		setSize(glm::vec3(resizeHandler(TextSize), 0));
	}
	else
	{
		setSize(glm::vec3(TextSize, 0));
	}
	if (repositionHandler)
	{
		position = repositionHandler(this->size);
	}
	oldText = text;
}
bool TextView::preRender()
{
	auto &model = getModelMatrix();
	auto shader = addShader();
	shader->bind(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, viewPointer ? viewPointer->matrix : scene.viewPointer->matrix);
	shader->setBlock("Projection", *this, projectionPointer ? projectionPointer->matrix : scene.projectionPointer->matrix);
	shader->setBlock("CameraPosition", *this, scene.viewPointer->position, 16);
	shader->setUniform("TextColor", *this, textColor);
	shader->unbind();
	return true;
}
void TextView::setSize(glm::vec3 size)
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