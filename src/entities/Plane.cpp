#include <zg/entities/Plane.hpp>
#include <zg/utilities.hpp>
using namespace zg::entities;
Plane::Plane(zg::Window &window,
             zg::Scene &scene,
             glm::vec3 position,
             glm::vec3 rotation,
             glm::vec3 scale,
             glm::vec2 size,
             glm::vec4 color,
             const zg::shaders::RuntimeConstants &constants,
             std::string_view name):
	zg::Entity(
		window,
		zg::mergeVectors<std::string_view>({
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
		scale,
		name.empty() ? "Plane " + std::to_string(++planesCount) : name
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
Plane::Plane(zg::Window &window,
             zg::Scene &scene,
             glm::vec3 position,
             glm::vec3 rotation,
             glm::vec3 scale,
             glm::vec2 size,
             textures::Texture &texture,
             const zg::shaders::RuntimeConstants &constants,
             std::string_view name):
	zg::Entity(
		window,
		zg::mergeVectors<std::string_view>({
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
		scale,
		name.empty() ? "Plane " + std::to_string(++planesCount) : name
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
void Plane::setColor(glm::vec4 color)
{
	colors = {color, color, color, color};
	updateElements("Color", colors);
};
void Plane::setSize(glm::vec2 size)
{
	positions = {
		{ -size.x / 2, -size. y / 2, 0}, { size.x / 2, -size. y / 2, 0}, { size.x / 2,  size. y / 2, 0}, { -size.x / 2,  size. y / 2, 0} // Frontm
	};
	updateElements("Position", positions);
	this->size = size;
};