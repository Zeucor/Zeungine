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
	fontSize(fontSize)
{
	computeNormals(indices, positions, normals);
	updateIndices(indices);
	updateElements("UV2", uvs);
	updateElements("Position", positions);
	updateElements("Normal", normals);
};
void TextView::update()
{
	if (oldText != text)
	{
		float lineHeight = 0;
		glm::vec3 cursorPosition(0);
		auto textSize = font.stringSize(text, fontSize, lineHeight, {0, 0});
		font.stringToTexture(text, {1, 1, 1, 1}, fontSize, lineHeight, textSize, texturePointer, 0, cursorPosition);
		oldText = text;
	}
};
void TextView::preRender()
{
  auto &model = getModelMatrix();
	shader.bind();
	shader.setBlock("Model", model);
	shader.setBlock("View", scene.view.matrix);
	shader.setBlock("Projection", scene.projection.matrix);
	shader.setBlock("CameraPosition", scene.view.position, 16);
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