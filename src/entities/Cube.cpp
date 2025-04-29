#include <zg/Scene.hpp>
#include <zg/Serial.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/utilities.hpp>
size_t cubesCount = 0;
zg::EntityCreateInfo zg::entities::CubeFactory(std::string _name, glm::vec3 position, glm::quat rotation,
																							 glm::vec3 scale, glm::vec3 size, glm::vec4 color,
																							 const shaders::RuntimeConstants& constants, zg::FRONTFACE frontFace)
{
	zg::EntityCreateInfo info{
		.typeName = "Cube",
		.position = position,
		.rotation = rotation,
		.scale = scale,
		.constants = zg::mergeVectors<std::string>(
			{{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
		.name = (_name.empty() ? "Cube " + std::to_string(++cubesCount) : _name),
		.indiceCount = [](auto&) { return 36; },
		.indices = [frontFace](auto&) -> std::vector<uint32_t> {
			if (frontFace == zg::CLOCKWISE)
				return {
					2,	1,	0,	0,	3,	2, // Front face
					6,	7,	4,	4,	5,	6, // Back face
					10, 9,	8,	8,	11, 10, // Left face
					14, 15, 12, 12, 13, 14, // Right face
					18, 17, 16, 16, 19, 18, // Top face
					22, 23, 20, 20, 21, 22 // Bottom face
				};
			else
				return {
					0,	1,	2,	2,	3,	0, // Front face
					4,	7,	6,	6,	5,	4, // Back face
					8,	9,	10, 10, 11, 8, // Left face
					12, 15, 14, 14, 13, 12, // Right face
					16, 17, 18, 18, 19, 16, // Top face
					20, 23, 22, 22, 21, 20 // Bottom face
				};
		},
		.vertexCount = [](auto&) { return 24; },
		.vertices = [](auto& entity) -> std::vector<glm::vec3>
		{
			auto& size = entity.template getData<glm::vec3>("Size");
			return {{
				{-size.x / 2, -size.y / 2, size.z / 2},	 {size.x / 2, -size.y / 2, size.z / 2},
				{size.x / 2, size.y / 2, size.z / 2},		 {-size.x / 2, size.y / 2, size.z / 2}, // Front
				{-size.x / 2, -size.y / 2, -size.z / 2}, {size.x / 2, -size.y / 2, -size.z / 2},
				{size.x / 2, size.y / 2, -size.z / 2},	 {-size.x / 2, size.y / 2, -size.z / 2}, // Back
				{-size.x / 2, -size.y / 2, -size.z / 2}, {-size.x / 2, -size.y / 2, size.z / 2},
				{-size.x / 2, size.y / 2, size.z / 2},	 {-size.x / 2, size.y / 2, -size.z / 2}, // Left
				{size.x / 2, -size.y / 2, -size.z / 2},	 {size.x / 2, -size.y / 2, size.z / 2},
				{size.x / 2, size.y / 2, size.z / 2},		 {size.x / 2, size.y / 2, -size.z / 2}, // Right
				{-size.x / 2, size.y / 2, -size.z / 2},	 {-size.x / 2, size.y / 2, size.z / 2},
				{size.x / 2, size.y / 2, size.z / 2},		 {size.x / 2, size.y / 2, -size.z / 2}, // Top
				{-size.x / 2, -size.y / 2, -size.z / 2}, {-size.x / 2, -size.y / 2, size.z / 2},
				{size.x / 2, -size.y / 2, size.z / 2},	 {size.x / 2, -size.y / 2, -size.z / 2} // Bottom
			}};
		},
		.colorCount = [](auto&){return 24;},
		.colors = [](auto& entity) -> std::vector<glm::vec4>
		{
			auto& color = entity.template getData<glm::vec4>("Color");
			return {24, color};
		},
		.dataMap = {
			{"Color", color},
			{"Size", size}
		}
	};
	return info;
}
