#include <zg/entities/Plane.hpp>
#include <zg/Mesh.hpp>
#include <zg/utilities.hpp>
using namespace zg;
zg::EntityCreateInfo zg::entities::PlaneFactory(glm::vec4 color, std::string name, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
    const shaders::RuntimeConstants constants, zg::FRONTFACE frontFace)
{
    auto mergedConstants = shaders::mergeConstants({
        shaders::RuntimeConstants({"Shape", "Color"}),
        shaders::common_zg_constants,
        constants
    });
    zg::MeshCreateInfo meshInfo{
        .name = "Plane",
        .shapeType = ShapeType::Plane,
        .material = {
            color,
            0
        },
        .info = [](auto&) -> MeshInfo {
            return  {};
        },
        .constants = mergedConstants
    };
    zg::EntityCreateInfo info{
        .typeName = "Plane",
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .name = name,
        .meshInfos = {
            meshInfo
        }
    };
    return info;
}
zg::EntityCreateInfo zg::entities::PlaneFactory(const std::shared_ptr<textures::Texture>& texture, std::string name, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
    const shaders::RuntimeConstants constants, zg::FRONTFACE frontFace)
{
    auto mergedConstants = shaders::mergeConstants({
        shaders::RuntimeConstants({"Shape", "UV2"}),
        shaders::common_zg_constants,
        constants
    });
    zg::MeshCreateInfo meshInfo{
        .name = "Plane",
        .shapeType = ShapeType::Plane,
        .material = {
            glm::vec4(1),
            1
        },
        .info = [](auto&) -> MeshInfo {
            return {
                .uv2s = {
                    {0, 0},
                    {1, 0},
                    {1, 1},
                    {0, 1}
                },
            };
        },
        .keyedTextures = {
            {"ColorTexture", texture}
        },
        .constants = mergedConstants
    };
    zg::EntityCreateInfo info{
        .typeName = "Plane",
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .name = name,
        .meshInfos = {
            meshInfo
        }
    };
    return info;
}   