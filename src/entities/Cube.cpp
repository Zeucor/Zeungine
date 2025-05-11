#include <zg/Scene.hpp>
#include <zg/Serial.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/utilities.hpp>
#include <zg/Mesh.hpp>
size_t cubesCount = 0;
zg::EntityCreateInfo zg::entities::CubeFactory(std::string _name, glm::vec3 position, glm::quat rotation,
							glm::vec3 scale, glm::vec4 color,
							const shaders::RuntimeConstants& constants, zg::FRONTFACE frontFace)
{
	zg::MeshCreateInfo meshInfo{
		.shapeType = ShapeType::Box,
		.material = {
			color,
			0
		},
		.constants = zg::mergeVectors<std::string>(
			{{"Shape", "Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants)
	};
	zg::EntityCreateInfo info{
		.typeName = "Cube",
		.position = position,
		.rotation = rotation,
		.scale = scale,
		.name = (_name.empty() ? "Cube " + std::to_string(++cubesCount) : _name),
		.meshInfos = { meshInfo }
	};
	return info;
}
