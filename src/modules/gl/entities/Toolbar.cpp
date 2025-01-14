#include <anex/modules/gl/entities/Plane.hpp>
#include <anex/modules/gl/entities/TextView.hpp>
#include <anex/modules/gl/entities/Toolbar.hpp>
using namespace anex::modules::gl::entities;
Toolbar::Toolbar(GLWindow &window,
                GLScene &scene,
                 const glm::vec3 &position,
                 const glm::vec3 &rotation,
                 const glm::vec3 &scale,
                 const glm::vec4 &color,
                 const float &height,
								 fonts::freetype::FreetypeFont &font):
	GLEntity(window, {
			"Color", "Position",
			"View", "Projection", "Model", "CameraPosition"
	}, 6, 4, position, rotation, scale),
	indices{
		0, 1, 2,  2, 3, 0   // Front face
	},
	colors({color, color, color, color}),
	positions({
		{ 0, -height, 0}, { window.windowWidth, -height, 0}, { window.windowWidth, 0, 0 }, { 0, 0, 0} // Front
	}),
	scene(scene),
	font(font)
{
	updateIndices(indices);
	updateElements("Color", colors.data());
	updateElements("Position", positions.data());
  auto xButton = std::make_shared<Plane>(window, scene, glm::vec3(window.windowWidth - 20, -20, 0.5), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::vec2(40, height), glm::vec4(1, 0, 0, 1));
  addChild(xButton);
  std::string xString("X");
  float xFontSize = 40.f;
  float xLineHeight = 0;
	auto textSize = font.stringSize(xString, xFontSize, xLineHeight, {0, 0});
  auto xTextView = std::make_shared<TextView>(window, scene, glm::vec3(0, 0, 0.5f), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), xString, textSize / 2.f, font, xFontSize);
  xButton->addChild(xTextView);
};
void Toolbar::preRender()
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