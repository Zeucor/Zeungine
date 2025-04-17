#include <zg/entities/Plane.hpp>
#include <zg/utilities.hpp>
using namespace zg::entities;
Plane::Plane(zg::Window &window, zg::Scene &scene, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
			 glm::vec2 size, glm::vec4 color, const zg::shaders::RuntimeConstants &constants, std::string_view name) : zg::Entity(window, scene,
																																  zg::mergeVectors<std::string>(
																																	  {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
																																  6, getIndices(window), 4,
																																  {
																																	  {-size.x / 2, -size.y / 2, 0},
																																	  {size.x / 2, -size.y / 2, 0},
																																	  {size.x / 2, size.y / 2, 0},
																																	  {-size.x / 2, size.y / 2, 0} // Front
																																  },
																																  position, rotation, scale, name.empty() ? "Plane " + std::to_string(++planesCount) : name),
																													   uvs({{}, {}, {}, {}}),  size(size)
{
	computeNormals(window.iRenderer->frontFace, indices, positions, normals);
	updateIndices(indices);
	setColor(color);
	updateElements("Position", positions);
	updateElements("Normal", normals);
};
Plane::Plane(zg::Window &window, zg::Scene &scene, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
			 glm::vec2 size, textures::Texture &texture, const zg::shaders::RuntimeConstants &constants,
			 std::string_view name) : zg::Entity(window, scene,
												 zg::mergeVectors<std::string>(
													 {{"UV2", "Position", "Normal", "Texture2D", "View", "Projection", "Model", "CameraPosition"}}, constants),
												 6,
												 getIndices(window),
												 4,
												 {
													 {-size.x / 2, -size.y / 2, 0},
													 {size.x / 2, -size.y / 2, 0},
													 {size.x / 2, size.y / 2, 0},
													 {-size.x / 2, size.y / 2, 0} // Front
												 },
												 position, rotation, scale, name.empty() ? "Plane " + std::to_string(++planesCount) : name),
									  colors({{}, {}, {}, {}}), uvs({
																	// Front face
																	{0, 0}, // 0
																	{1, 0}, // 1
																	{1, 1}, // 2
																	{0, 1}	// 3
																}),
									   texturePointer(&texture), size(size)
{
	switch (window.iRenderer->renderer)
	{
	default:
		break;
	case RENDERER_VULKAN:
	case RENDERER_METAL:
		flipUVsY(uvs);
		break;
	}
	computeNormals(window.iRenderer->frontFace, indices, positions, normals);
	updateIndices(indices);
	updateElements("UV2", uvs);
	updateElements("Position", positions);
	updateElements("Normal", normals);
};
std::vector<uint32_t> Plane::getIndices(zg::Window& window)
{
	if (window.iRenderer->frontFace == zg::CLOCKWISE)
		return {{0, 1, 2, 2, 3, 0}};
	else
		return {{2, 1, 0, 0, 3, 2}};
}
bool Plane::preRender()
{
	const auto &model = getModelMatrix();
	auto shader = addShader();
	shader->bind(*this);
	scene.entityPreRender(*this);
	shader->setBlock("Model", *this, model);
	shader->setBlock("View", *this, viewPointer ? viewPointer->matrix : scene.viewPointer->matrix);
	shader->setBlock("Projection", *this, projectionPointer ? projectionPointer->matrix : scene.projectionPointer->matrix);
	shader->setBlock("CameraPosition", *this, scene.viewPointer->position, 16);
	if (texturePointer)
		shader->setTexture("Texture2D", *this, *texturePointer, 0);
	shader->unbind();
	return true;
};
void Plane::setColor(glm::vec4 color)
{
	colors = {color, color, color, color};
	updateElements("Color", colors);
};
void Plane::setSize(glm::vec2 size)
{
	positions = {
		{-size.x / 2, -size.y / 2, 0},
		{size.x / 2, -size.y / 2, 0},
		{size.x / 2, size.y / 2, 0},
		{-size.x / 2, size.y / 2, 0} // Front
	};
	updateElements("Position", positions);
	this->size = size;
};

template<>
Serial& serialize(Serial& serial, const std::shared_ptr<zg::entities::Plane>& planePointer)
{
	if (!planePointer)
	{
		serial << false;
		return serial;
	}
	auto& plane = *planePointer;
	serial << true;
	auto& position = plane.position;
	auto& rotation = plane.rotation;
	auto& scale = plane.scale;
	auto& size = plane.size;
	auto& color = plane.color;
	auto& constants = plane.constants;
	auto& name = plane.name;
	serial << position << rotation << scale << size;
	auto constantsSize = constants.size();
	serial << constantsSize;
	for (auto j = 0; j < constantsSize; j++)
		serial << constants[j];
	serial << name;
	return serial;
}
template<>
Serial& deserialize(Serial& serial, std::shared_ptr<zg::entities::Plane>& planePointer)
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
	glm::vec2 size(0);
	glm::vec4 color(0);
	zg::shaders::RuntimeConstants constants{};
	std::string name{};
	serial >> position >> rotation >> scale >> size >> color;
	auto constantsSize = constants.size();
	serial >> constantsSize;
	constants.resize(constantsSize);
	for (auto j = 0; j < constantsSize; j++)
		serial >> constants[j];
	serial >> name;
	planePointer = std::make_shared<zg::entities::Plane>(*windowPointer, *scenePointer, position, rotation, scale, size, color, constants, name);
	return serial;
}