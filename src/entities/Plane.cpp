#include <zg/entities/Plane.hpp>
#include <zg/Mesh.hpp>
#include <zg/utilities.hpp>
using namespace zg;
zg::EntityCreateInfo zg::entities::PlaneFactory(glm::vec4 color, std::string name, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec2 size,
    const shaders::RuntimeConstants& constants, zg::FRONTFACE frontFace)
{
    zg::MeshCreateInfo meshInfo{
        .indiceCount = [](auto&) { return 6; },
        .indices = [frontFace](auto&) -> std::vector<uint32_t>
        {
            if (frontFace == zg::CLOCKWISE)
                return {
                    2,	1,	0,	0,	3,	2, // Front face
                };
            else
                return {
                    0,	1,	2,	2,	3,	0, // Front face
                };
        },
        .vertexCount = [](auto&) { return 4; },
        .vertices = [](auto& entity) -> std::vector<glm::vec3>
        {
            auto& size = entity.template getData<glm::vec2>("Size");           
            return {{
                {-size.x / 2, -size.y / 2, 0},	 {size.x / 2, -size.y / 2, 0},
                {size.x / 2, size.y / 2, 0},		 {-size.x / 2, size.y / 2, 0} // Front
            }};
        },
        .colorCount = [](auto&) { return 4; },
        .colors = [](auto& entity) -> std::vector<glm::vec4>
        {
            auto& color = entity.template getData<glm::vec4>("Color");
            return {4, color};
        },
        .constants = zg::mergeVectors<std::string>(
            {{"Color", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants)
    };
    zg::EntityCreateInfo info{
        .typeName = "Plane",
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .name = name,
        .dataMap = {
            {"Color", color},
            {"Size", size}
        },
        .meshInfos = {
            meshInfo
        }
    };
    return info;
}
zg::EntityCreateInfo zg::entities::PlaneFactory(const std::shared_ptr<textures::Texture>& texture, std::string name, glm::vec3 position, glm::quat rotation, glm::vec3 scale, glm::vec2 size,
    const shaders::RuntimeConstants& constants, zg::FRONTFACE frontFace)
{
    zg::MeshCreateInfo meshInfo{
        .indiceCount = [](auto&) { return 6; },
        .indices = [frontFace](auto&) -> std::vector<uint32_t>
        {
            if (frontFace == zg::CLOCKWISE)
                return {
                    2,	1,	0,	0,	3,	2, // Front face
                };
            else
                return {
                    0,	1,	2,	2,	3,	0, // Front face
                };
        },
        .vertexCount = [](auto&) { return 4; },
        .vertices = [](auto& entity) -> std::vector<glm::vec3>
        {
            auto& size = entity.template getData<glm::vec2>("Size");           
            return {{
                {-size.x / 2, -size.y / 2, 0},	 {size.x / 2, -size.y / 2, 0},
                {size.x / 2, size.y / 2, 0},		 {-size.x / 2, size.y / 2, 0} // Front
            }};
        },
        .uv2Count = [](auto&) { return 4; },
        .uv2s = [](auto&) -> std::vector<glm::vec2>
        {
            return {
                {0, 0},
                {1, 0},
                {1, 1},
                {0, 1}
            };
        },
        .keyedTextures = {
            {"ColorTexture", texture}
        },
        .constants = zg::mergeVectors<std::string>(
			{{"UV2", "Position", "Normal", "ColorTexture", "View", "Projection", "Model", "CameraPosition"}}, constants)
    };
    zg::EntityCreateInfo info{
        .typeName = "Plane",
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .name = name,
        .dataMap = {
            {"Size", size}
        },
        .meshInfos = {
            meshInfo
        }
    };
    return info;
}   