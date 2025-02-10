#include <zg/Scene.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/utilities.hpp>
using namespace zg::entities;
Cube::Cube(zg::Window& window, zg::Scene& scene, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale,
					 glm::vec3 size, const zg::shaders::RuntimeConstants& constants, std::string_view name) :
		zg::Entity(window,
							 zg::mergeVectors<std::string_view>(
								 {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
							 36,
							 {
								 0,	 1,	 2,	 2,	 3,	 0, // Front face
								 4,	 7,	 6,	 6,	 5,	 4, // Back face
								 8,	 9,	 10, 10, 11, 8, // Left face
								 12, 15, 14, 14, 13, 12, // Right face
								 16, 17, 18, 18, 19, 16, // Top face
								 20, 23, 22, 22, 21, 20 // Bottom face
							 },
							 24,
							 {
								 {-size.x / 2, -size.y / 2, size.z / 2},	{size.x / 2, -size.y / 2, size.z / 2},
								 {size.x / 2, size.y / 2, size.z / 2},		{-size.x / 2, size.y / 2, size.z / 2}, // Front
								 {-size.x / 2, -size.y / 2, -size.z / 2}, {size.x / 2, -size.y / 2, -size.z / 2},
								 {size.x / 2, size.y / 2, -size.z / 2},		{-size.x / 2, size.y / 2, -size.z / 2}, // Back
								 {-size.x / 2, -size.y / 2, -size.z / 2}, {-size.x / 2, -size.y / 2, size.z / 2},
								 {-size.x / 2, size.y / 2, size.z / 2},		{-size.x / 2, size.y / 2, -size.z / 2}, // Left
								 {size.x / 2, -size.y / 2, -size.z / 2},	{size.x / 2, -size.y / 2, size.z / 2},
								 {size.x / 2, size.y / 2, size.z / 2},		{size.x / 2, size.y / 2, -size.z / 2}, // Right
								 {-size.x / 2, size.y / 2, -size.z / 2},	{-size.x / 2, size.y / 2, size.z / 2},
								 {size.x / 2, size.y / 2, size.z / 2},		{size.x / 2, size.y / 2, -size.z / 2}, // Top
								 {-size.x / 2, -size.y / 2, -size.z / 2}, {-size.x / 2, -size.y / 2, size.z / 2},
								 {size.x / 2, -size.y / 2, size.z / 2},		{size.x / 2, -size.y / 2, -size.z / 2} // Bottom
							 },
							 position, rotation, scale, name.empty() ? "Cube " + std::to_string(++cubesCount) : name),
		colors({
			{1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 0, 1}, // Front face
			{0, 1, 0, 1}, {0, 1, 0, 1}, {0, 1, 0, 1}, {0, 1, 0, 1}, // Back face
			{0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, // Left face
			{1, 1, 0, 1}, {1, 1, 0, 1}, {1, 1, 0, 1}, {1, 1, 0, 1}, // Right face
			{0, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1}, {0, 1, 1, 1}, // Top face
			{1, 0, 1, 1}, {1, 0, 1, 1}, {1, 0, 1, 1}, {1, 0, 1, 1} // Bottom face
		}),
		scene(scene)
{
	computeNormals(indices, positions, normals);
	updateIndices(indices);
	updateElements("Color", colors);
	updateElements("Position", positions);
	updateElements("Normal", normals);
};
bool Cube::preRender()
{
	const auto& model = getModelMatrix();
	shader->bind(*this);
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, scene.view.matrix);
	shader->setBlock("Projection", *this, scene.projection.matrix);
	shader->setBlock("CameraPosition", *this, scene.view.position, 16);
	shader->unbind();
	return true;
};
