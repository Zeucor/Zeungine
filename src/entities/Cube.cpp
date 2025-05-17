#include <zg/Scene.hpp>
#include <zg/Serial.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/utilities.hpp>
#include <zg/Mesh.hpp>
size_t cubesCount = 0;
using namespace zg;
EntityCreateInfo entities::CubeFactory(std::string _name, glm::vec3 position, glm::quat rotation,
							glm::vec3 scale, glm::vec4 color,
							const shaders::RuntimeConstants constants, FRONTFACE frontFace)
{
    auto mergedConstants = shaders::mergeConstants({
        shaders::RuntimeConstants({"Shape", "Color"}),
        shaders::common_zg_constants,
        constants
	});
	MeshCreateInfo meshInfo{
		.name = "Cube",
		.shapeType = ShapeType::Box,
		.material = {
			color,
			0
		},
		.info = [](auto&) -> MeshInfo {
			return { };
		},
		.constants = mergedConstants
	};
	EntityCreateInfo info{
		.typeName = "Cube",
		.position = position,
		.rotation = rotation,
		.scale = scale,
		.name = (_name.empty() ? "Cube " + std::to_string(++cubesCount) : _name),
		.meshInfos = { meshInfo }
	};
	return info;
}
