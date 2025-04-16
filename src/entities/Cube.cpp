#include <zg/Scene.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/utilities.hpp>
#include <zg/Serial.hpp>
using namespace zg::entities;
Cube::Cube(zg::Window& window, zg::Scene& scene, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
					 glm::vec3 size, const zg::shaders::RuntimeConstants& constants, std::string_view name) :
		zg::Entity(window, scene,
							 zg::mergeVectors<std::string>(
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
		size(size),
		colors({
			{1, 0, 0, 1}, {1, 0, 0, 1}, {1, 0, 1, 1}, {1, 0, 1, 1}, // Front face
			{0, 0, 1, 1}, {0, 0, 1, 1}, {0, 1, 0, 1}, {0, 1, 0, 1}, // Back face
			{0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, {0, 0, 1, 1}, // Left face
			{1, 1, 0, 1}, {1, 1, 0, 1}, {1, 1, 0, 1}, {1, 1, 0, 1}, // Right face
			{0, 0.2, 1, 1}, {0, 1, 0.3, 1}, {0.5, 0, 1, 1}, {.8, 0.7, 0, 1}, // Top face
			{1, 0, 1, 1}, {1, 0, 1, 1}, {1, 0, 1, 1}, {1, 0, 1, 1} // Bottom face
		})
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
	auto shader = addShader();
	shader->bind(*this);
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, viewPointer ? viewPointer->matrix : scene.viewPointer->matrix);
	shader->setBlock("Projection", *this, projectionPointer ? projectionPointer->matrix : scene.projectionPointer->matrix);
	shader->setBlock("CameraPosition", *this, scene.viewPointer->position, 16);
	shader->unbind();
	return true;
}
template<>
Serial& serialize(Serial& serial, const std::shared_ptr<zg::entities::Cube>& cubePointer)
{
	if (!cubePointer)
	{
		serial << false;
		return serial;
	}
	auto& cube = *cubePointer;
	serial << true;
	auto& position = cube.position;
	auto& rotation = cube.rotation;
	auto& scale = cube.scale;
	auto& size = cube.size;
	auto& constants = cube.constants;
	auto& name = cube.name;
	serial << position << rotation << scale << size;
	auto constantsSize = constants.size();
	serial << constantsSize;
	for (auto j = 0; j < constantsSize; j++)
		serial << constants[j];
	serial << name;
	return serial;
}
template<>
Serial& deserialize(Serial& serial, std::shared_ptr<zg::entities::Cube>& cubePointer)
{
	bool wroteBit = false;
	serial >> wroteBit;
	if (!wroteBit)
		return serial;
	zg::Window* windowPointer = (zg::Window*)serial.getContextPointer("Window");
	zg::Scene* scenePointer = (zg::Scene*)serial.getContextPointer("Scene");
	glm::vec3 position(0);
	glm::quat rotation(0, 0, 0, 0);
	glm::vec3 scale(0);
	glm::vec3 size(0);
	zg::shaders::RuntimeConstants constants{};
	std::string name{};
	serial >> position >> rotation >> scale >> size;
	auto constantsSize = constants.size();
	serial >> constantsSize;
	constants.resize(constantsSize);
	for (auto j = 0; j < constantsSize; j++)
		serial >> constants[j];
	serial >> name;
	cubePointer = std::make_shared<zg::entities::Cube>(*windowPointer, *scenePointer, position, rotation, scale, size, constants, name);
	return serial;
}