#include <zg/entities/Plane.hpp>
#include <zg/utilities.hpp>
using namespace zg;
zg::EntityCreateInfo zg::entities::PlaneFactory(glm::vec4 color, std::string name, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec2 size,
    const shaders::RuntimeConstants& constants, zg::FRONTFACE frontFace)
{
	std::vector<glm::vec4> colors(4, color);
	std::vector<glm::vec3> vertices({{
		{-size.x / 2, -size.y / 2, 0},	 {size.x / 2, -size.y / 2, 0},
		{size.x / 2, size.y / 2, 0},		 {-size.x / 2, size.y / 2, 0} // Front
	}});
	std::vector<uint32_t> indices;
	if (frontFace == zg::CLOCKWISE)
		indices = {
			2,	1,	0,	0,	3,	2, // Front face
		};
	else
		indices = {
			0,	1,	2,	2,	3,	0, // Front face
		};
    zg::EntityCreateInfo info{
        .typeName = "Plane",
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .constants = zg::mergeVectors<std::string>(
			{{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants),
        .name = name,
        .indiceCount = 6,
        .indices = indices,
        .vertexCount = 4,
        .vertices = vertices,
        .colors = colors
    };
    return info;
}
zg::EntityCreateInfo zg::entities::PlaneFactory(textures::Texture& texture, std::string name, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec2 size,
    const shaders::RuntimeConstants& constants, zg::FRONTFACE frontFace)
{
	std::vector<glm::vec2> uv2s({
        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1}
    });
	std::vector<glm::vec3> vertices({{
		{-size.x / 2, -size.y / 2, 0},	 {size.x / 2, -size.y / 2, 0},
		{size.x / 2, size.y / 2, 0},		 {-size.x / 2, size.y / 2, 0} // Front
	}});
	std::vector<uint32_t> indices;
	if (frontFace == zg::CLOCKWISE)
		indices = {
			2,	1,	0,	0,	3,	2, // Front face
		};
	else
		indices = {
			0,	1,	2,	2,	3,	0, // Front face
		};
    zg::EntityCreateInfo info{
        .typeName = "Plane",
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .constants = zg::mergeVectors<std::string>(
			{{"UV2", "Position", "Normal", "Texture2D", "View", "Projection", "Model", "CameraPosition"}}, constants),
        .name = name,
        .indiceCount = 6,
        .indices = indices,
        .vertexCount = 4,
        .vertices = vertices,
        .uv2s = uv2s
    };
    return info;
    // 		texturePointer(&texture), size(size)
}