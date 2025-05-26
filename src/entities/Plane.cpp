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
        .shapeType = ShapeType::PlaneXY,
        .material = {
            color,
            0
        },
        .info = [a = color.a](auto& entity) -> MeshInfo {
            ((Entity&)entity).isTransparent = (a < 1.f);
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
        shaders::RuntimeConstants({"Shape", "UV2", "ColorTexture"}),
        shaders::common_zg_constants,
        constants
    });
    zg::MeshCreateInfo meshInfo{
        .name = "Plane",
        .shapeType = ShapeType::PlaneXY,
        .material = {
            glm::vec4(1),
            1
        },
        .info = [](auto&) -> MeshInfo {
            return { };
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