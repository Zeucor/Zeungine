#include <anex/modules/gl/entities/Cube.hpp>
#include <anex/utilities.hpp>
using namespace anex::modules::gl::entities;
Cube::Cube(anex::modules::gl::GLWindow &window, anex::modules::gl::GLScene &scene, const glm::vec3 &position, const glm::vec3 &rotation, const glm::vec3 &scale, const glm::vec3 &size, const anex::modules::gl::shaders::RuntimeConstants &constants):
	anex::modules::gl::GLEntity(
		window,
		anex::mergeVectors<std::string_view>({
			{
				"Color", "Position", "Normal",
				"View", "Projection", "Model", "CameraPosition"
			}
		}, constants),
		36,
    24,
		position,
		rotation,
		scale
	),
	indices{
		0, 1, 2,  2, 3, 0,   // Front face
		4, 7, 6,  6, 5, 4,   // Back face
		8, 9, 10, 10, 11, 8,  // Left face
		12, 15, 14, 14, 13, 12, // Right face
		16, 17, 18, 18, 19, 16, // Top face
		20, 23, 22, 22, 21, 20  // Bottom face
	},
	colors({{1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, // Front face
					{0, 1, 0, 1}, {0, 1, 0, 1}, {0, 1, 0, 1}, {0, 1, 0, 1}, // Back face
					{0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, // Left face
					{1, 1, 0, 1}, {1, 1, 0, 1}, {1, 1, 0, 1}, {1, 1, 0, 1}, // Right face
					{0, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1}, // Top face
					{1, 0, 1, 1}, {1, 0, 1, 1}, {1, 0, 1, 1}, {1, 0, 1, 1} // Bottom face
				 }),
	positions({
		{ -size.x / 2, -size. y / 2,  size.z / 2}, {  size.x / 2, -size. y / 2,  size.z / 2}, {  size.x / 2,  size. y / 2,  size.z / 2}, { -size.x / 2,  size. y / 2,  size.z / 2}, // Front
		{ -size.x / 2, -size. y / 2, -size.z / 2}, {  size.x / 2, -size. y / 2, -size.z / 2}, {  size.x / 2,  size. y / 2, -size.z / 2}, { -size.x / 2,  size. y / 2, -size.z / 2}, // Back
		{ -size.x / 2, -size. y / 2, -size.z / 2}, { -size.x / 2, -size. y / 2,  size.z / 2}, { -size.x / 2,  size. y / 2,  size.z / 2}, { -size.x / 2,  size. y / 2, -size.z / 2}, // Left
		{  size.x / 2, -size. y / 2, -size.z / 2}, {  size.x / 2, -size. y / 2,  size.z / 2}, {  size.x / 2,  size. y / 2,  size.z / 2}, {  size.x / 2,  size. y / 2, -size.z / 2}, // Right
		{ -size.x / 2,  size. y / 2, -size.z / 2}, { -size.x / 2,  size. y / 2,  size.z / 2}, {  size.x / 2,  size. y / 2,  size.z / 2}, {  size.x / 2,  size. y / 2, -size.z / 2}, // Top
		{ -size.x / 2, -size. y / 2, -size.z / 2}, { -size.x / 2, -size. y / 2,  size.z / 2}, {  size.x / 2, -size. y / 2,  size.z / 2}, {  size.x / 2, -size. y / 2, -size.z / 2}  // Bottom
	}),
	scene(scene)
{
	computeNormals(indiceCount, indices, positions, normals);
	updateIndices(indices);
	updateElements("Color", colors.data());
	updateElements("Position", positions.data());
	updateElements("Normal", normals.data());
};
void Cube::preRender()
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