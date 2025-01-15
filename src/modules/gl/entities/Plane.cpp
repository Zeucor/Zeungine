#include <anex/modules/gl/entities/Plane.hpp>
#include <anex/utilities.hpp>
using namespace anex::modules::gl::entities;
Plane::Plane(anex::modules::gl::GLWindow &window,
             anex::modules::gl::GLScene &scene,
             const glm::vec3 &position,
             const glm::vec3 &rotation,
             const glm::vec3 &scale,
             const glm::vec2 &size,
             const glm::vec4 &color,
             const anex::modules::gl::shaders::RuntimeConstants &constants):
	anex::modules::gl::GLEntity(
		window,
		anex::mergeVectors<std::string_view>({
			{
				"Color", "Position", "Normal",
				"View", "Projection", "Model", "CameraPosition"
			}
		}, constants),
		6,
		{0, 1, 2,  2, 3, 0},
    4,
		{
			{ -size.x / 2, -size. y / 2, 0}, { size.x / 2, -size. y / 2, 0}, { size.x / 2,  size. y / 2, 0}, { -size.x / 2,  size. y / 2, 0} // Frontm
		},
		position,
		rotation,
		scale
	),
	uvs({{},{},{},{}}),
	scene(scene),
	size(size)
{
	computeNormals(indices, positions, normals);
	updateIndices(indices);
	setColor(color);
	updateElements("Position", positions);
	updateElements("Normal", normals);
};
Plane::Plane(anex::modules::gl::GLWindow &window,
             anex::modules::gl::GLScene &scene,
             const glm::vec3 &position,
             const glm::vec3 &rotation,
             const glm::vec3 &scale,
             const glm::vec2 &size,
             textures::Texture &texture,
             const anex::modules::gl::shaders::RuntimeConstants &constants):
	anex::modules::gl::GLEntity(
		window,
		anex::mergeVectors<std::string_view>({
			{
				"UV2", "Position", "Normal", "Texture2D",
				"View", "Projection", "Model", "CameraPosition"
			}
		}, constants),
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
		scale
	),
	colors({{}, {}, {}, {}}),
	uvs({
		// Front face
		{ 0, 0 },  // 0
		{ 1, 0 },  // 1
		{ 1, 1 },  // 2
		{ 0, 1 }  // 3
	}),
	scene(scene),
	texturePointer(&texture),
	size(size)
{
	computeNormals(indices, positions, normals);
	updateIndices(indices);
	updateElements("UV2", uvs);
	updateElements("Position", positions);
	updateElements("Normal", normals);
};
void Plane::preRender()
{
	const auto &model = getModelMatrix();
	shader.bind();
	scene.entityPreRender(*this);
	shader.setBlock("Model", model);
	shader.setBlock("View", scene.view.matrix);
	shader.setBlock("Projection", scene.projection.matrix);
	shader.setBlock("CameraPosition", scene.view.position, 16);
  if (texturePointer)
	  shader.setTexture("Texture2D", *texturePointer, 0);
	shader.unbind();
};
void Plane::setColor(const glm::vec4 &color)
{
	colors = {color, color, color, color};
	updateElements("Color", colors);
};
void Plane::setSize(const glm::vec2 &size)
{
	positions = {
		{ -size.x / 2, -size. y / 2, 0}, { size.x / 2, -size. y / 2, 0}, { size.x / 2,  size. y / 2, 0}, { -size.x / 2,  size. y / 2, 0} // Frontm
	};
	updateElements("Position", positions);
	this->size = size;
};