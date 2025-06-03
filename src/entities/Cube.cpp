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
std::vector<glm::vec3> zg::entities::getCubeVertices()
{
  // Define the 8 unique corners of the cube
  glm::vec3 v0(-0.5, -0.5, -0.5); // NLL
  glm::vec3 v1( 0.5, -0.5, -0.5); // NLR
  glm::vec3 v2(-0.5,  0.5, -0.5); // NUL
  glm::vec3 v3( 0.5,  0.5, -0.5); // NUR
  glm::vec3 v4(-0.5, -0.5,  0.5); // FLL
  glm::vec3 v5( 0.5, -0.5,  0.5); // FLR
  glm::vec3 v6(-0.5,  0.5,  0.5); // FUL
  glm::vec3 v7( 0.5,  0.5,  0.5); // FUR

  return {
    	v0,
    	v1,
    	v2,
    	v1,
    	v3,
    	v2,

		v5,
		v4,
		v7,
		v4,
		v6,
		v7,
		
		v4,
		v0,
		v6,
		v0,
		v2,
		v6,
		
		v1,
		v5,
		v3,
		v5,
		v7,
		v3,

		v2,
		v3,
		v6,
		v3,
		v7,
		v6,
		
		v4,
		v5,
		v0,
		v5,
		v1,
		v0
	};
}
std::vector<uint32_t> zg::entities::getCubeIndices()
{
	return {
		// Front
		0,
		1,
		2,
		3,
		4,
		5,
		// Back
		6,
		7,
		8,
		9,
		10,
		11,
		// Left
		12,
		13,
		14,
		15,
		16,
		17,
		// Right
		18,
		19,
		20,
		21,
		22,
		23,
		// Top
		24,
		25,
		26,
		27,
		28,
		29,
		// Bottom
		30,
		31,
		32,
		33,
		34,
		35
	};
}