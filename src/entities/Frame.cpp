#include <zg/entities/Frame.hpp>
#include <zg/utilities.hpp>
using namespace zg::entities;
// Frame::Frame(zg::Window& window, zg::Scene& scene, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
// 						 glm::vec2 size, float borderWidth, glm::vec4 color, const zg::shaders::RuntimeConstants& constants,
// 						 std::string_view name) :
// 		zg::Entity(window, scene,
// 							 zg::mergeVectors<std::string>(
// 								 {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
// 							 getIndiceCount(), getIndices(window), getvertexCount(), getElements(size, borderWidth), position,
// 							 rotation, scale, name.empty() ? "Frame " + std::to_string(++framesCount) : name),
// 		uvs({{}, {}, {}, {}}), size(size)
// {
// 	updateIndices(indices);
// 	setColor(color);
// 	updateElements("Position", vertices);
// };
// std::vector<uint32_t> Frame::getIndices(zg::Window& window)
// {
// 	if (window.iRenderer->frontFace == zg::CLOCKWISE)
// 		return {{
// 			0, 1, 5, 4, 0, 5, // right
// 			5, 1, 2, 2, 6, 5, // bottom
// 			7, 6, 2, 3, 7, 2, // left
// 			3, 0, 7, 7, 0, 4 // top
// 		}};
// 	else
// 		return {{
// 			5, 1, 0, 5, 0, 4, // right
// 			2, 1, 5, 5, 6, 2, // bottom
// 			2, 6, 7, 2, 7, 3, // left
// 			7, 0, 3, 4, 0, 7 // top
// 		}};
// }
// std::vector<glm::vec3> Frame::getElements(glm::vec2 size, float borderWidth)
// {
// 	return {
// 		{(size.x / 2.f), (size.y / 2.f), 0},
// 		{(size.x / 2.f), -(size.y / 2.f), 0},
// 		{-(size.x / 2.f), -(size.y / 2.f), 0},
// 		{-(size.x / 2.f), (size.y / 2.f), 0},
// 		{(size.x / 2.f) - borderWidth, (size.y / 2.f) - borderWidth, 0},
// 		{(size.x / 2.f) - borderWidth, -(size.y / 2.f) + borderWidth, 0},
// 		{-(size.x / 2.f) + borderWidth, -(size.y / 2.f) + borderWidth, 0},
// 		{-(size.x / 2.f) + borderWidth, (size.y / 2.f) - borderWidth, 0},
// 	};
// }
// uint32_t Frame::getIndiceCount() { return 8 * 3; }
// uint32_t Frame::getvertexCount() { return 8; }
// void Frame::setColor(glm::vec4 color)
// {
// 	colors.resize(vertices.size());
// 	for (auto& thisColor : colors)
// 		thisColor = color;
// 	updateElements("Color", colors);
// };
// void Frame::setSize(glm::vec2 size)
// {
// 	this->size = size;
// 	vertices = getElements(this->size, borderWidth);
// 	updateElements("Position", vertices);
// };

// template <>
// Serial& serialize(Serial& serial, const std::shared_ptr<zg::entities::Frame>& framePointer)
// {
// 	if (!framePointer)
// 	{
// 		serial << false;
// 		return serial;
// 	}
// 	auto& frame = *framePointer;
// 	serial << true;
// 	auto& position = frame.position;
// 	auto& rotation = frame.rotation;
// 	auto& scale = frame.scale;
// 	auto& size = frame.size;
// 	auto& borderWidth = frame.borderWidth;
// 	auto& color = frame.color;
// 	auto& constants = frame.constants;
// 	auto& name = frame.name;
// 	serial << position << rotation << scale << size << borderWidth;
// 	auto constantsSize = constants.size();
// 	serial << constantsSize;
// 	for (auto j = 0; j < constantsSize; j++)
// 		serial << constants[j];
// 	serial << name;
// 	return serial;
// }
// template <>
// Serial& deserialize(Serial& serial, std::shared_ptr<zg::entities::Frame>& framePointer)
// {
// 	bool wroteBit = false;
// 	serial >> wroteBit;
// 	if (!wroteBit)
// 		return serial;
// 	zg::Window* windowPointer = (zg::Window*)serial.getContextPointer("Window");
// 	zg::Scene* scenePointer = (zg::Scene*)serial.getContextPointer("Scene");
// 	glm::vec3 position(0);
// 	glm::quat rotation(0, 0, 0, 0);
// 	glm::vec3 scale(0);
// 	glm::vec2 size(0);
// 	float borderWidth = 0.f;
// 	glm::vec4 color(0);
// 	zg::shaders::RuntimeConstants constants{};
// 	std::string name{};
// 	serial >> position >> rotation >> scale >> size >> borderWidth >> color;
// 	auto constantsSize = constants.size();
// 	serial >> constantsSize;
// 	constants.resize(constantsSize);
// 	for (auto j = 0; j < constantsSize; j++)
// 		serial >> constants[j];
// 	serial >> name;
// 	framePointer = std::make_shared<zg::entities::Frame>(*windowPointer, *scenePointer, position, rotation, scale, size,
// 																											 borderWidth, color, constants, name);
// 	return serial;
// }
