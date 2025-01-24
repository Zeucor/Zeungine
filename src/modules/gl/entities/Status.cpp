#include <anex/modules/gl/entities/Status.hpp>
#include <anex/utilities.hpp>
using namespace anex::modules::gl::entities;
Status::Status(anex::modules::gl::GLWindow &window,
	             anex::modules::gl::GLScene &scene,
	             glm::vec3 position,
	             glm::vec3 rotation,
	             glm::vec3 scale,
	             glm::vec4 color,
							 fonts::freetype::FreetypeFont& font,
							 float width,
							 float height,
							 std::string_view text,
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
		name.empty() ? "Status " + std::to_string(++statusesCount) : name
	),
	scene(scene),
	size({ 0, 0 }),
  color(color),
	font(font),
	width(width),
	height(height),
	NDCHeight((height / window.windowHeight) * 2)
{
	updateIndices(indices);
	setColor(color);
	setSize();
	float FontSize = height / 1.5f;
	float LineHeight = 0;
	auto TextSize = font.stringSize(text, FontSize, LineHeight, {0, 0});
	TextSize.y /= window.windowHeight * 0.5f;
	TextSize.x /= window.windowWidth * 0.5f;
	textView = std::make_shared<TextView>(
		window,
		scene,
		glm::vec3(0),
		glm::vec3(0),
		glm::vec3(1),
		glm::vec4(1, 1, 1, 1),
		text,
		TextSize,
		font,
		FontSize,
		true,
		[&](auto textSize)
		{
			return glm::vec3(textSize.x / 2 + 8 / this->window.windowWidth / 0.5, -NDCHeight / 2, 0.1);
		},
		TextView::ResizeHandler(),
		[&]
		{
			return NDCHeight * this->window.windowHeight * 0.5f / 1.5f;
		});
	textView->addToBVH = false;
	addChild(textView);
  addToBVH = false;
}
void Status::preRender()
{
	const auto &model = getModelMatrix();
	shader.bind();
	scene.entityPreRender(*this);
	shader.setBlock("Model", model);
	shader.setBlock("View", scene.view.matrix);
	shader.setBlock("Projection", scene.projection.matrix);
	shader.setBlock("CameraPosition", scene.view.position, 16);
	shader.unbind();
}
void Status::setColor(glm::vec4 newColor)
{
	colors = {newColor, newColor, newColor, newColor};
	updateElements("Color", colors);
};
void Status::setSize()
{
	glm::vec2 newSize(width / this->window.windowWidth / 0.5, height / this->window.windowHeight / 0.5);
	positions = {
		{ 0, -newSize.y, 0 }, { newSize.x, -newSize.y, 0 }, { newSize.x, 0, 0 }, { 0, 0, 0 }
	};
	updateElements("Position", positions);
	this->size = newSize;
}
void Status::setText(std::string_view text)
{
	textView->text = text;
}
void Status::setTextColor(glm::vec4 newTextColor)
{
	textView->setTextColor(newTextColor);
}