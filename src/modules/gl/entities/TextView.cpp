#include <anex/modules/gl/entities/TextView.hpp>
#include <anex/utilities.hpp>
using namespace anex::modules::gl::entities;
TextView::TextView(GLWindow &window,
                   GLScene &scene,
                   const glm::vec3 &position,
                   const glm::vec3 &rotation,
                   const glm::vec3 &scale,
                   const std::string &text,
                   const glm::vec2 &size,
                   fonts::freetype::FreetypeFont &font,
                   const float &fontSize,
                   const bool &textSizeIsNDC,
                   const RepositionHandler &repositionHandler,
                   const ResizeHandler &resizeHandler,
                   const ReFontSizeHandler &reFontSizeHandler,
                   const std::string &name):
	GLEntity(window,
		{
			{
				"UV2", "Position", "Normal", "Texture2D",
				"View", "Projection", "Model", "CameraPosition"
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
};
void TextView::update()
{
	if (oldText != text)
	{
		forceUpdate();
	}
};
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
		auto &glWindow = ((VAO &)*this).window;
		TextSize.x /= glWindow.windowWidth * 0.5;
		TextSize.y /= glWindow.windowHeight * 0.5;
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
};
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
	shader.unbind();
	shader.unbind();
};
void TextView::setSize(const glm::vec2 &size)
{
	this->size = size;
	positions = {
		{ -size.x / 2, -size. y / 2, 0}, { size.x / 2, -size. y / 2, 0}, { size.x / 2,  size. y / 2, 0}, { -size.x / 2,  size. y / 2, 0} // Frontm
	};
	updateElements("Position", positions);
};
void TextView::updateText(const std::string &text)
{
	this->text = text;
};