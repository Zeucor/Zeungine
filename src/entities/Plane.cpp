#include <zg/entities/Plane.hpp>
#include <zg/Mesh.hpp>
#include <zg/utilities.hpp>
using namespace zg;
zg::EntityCreateInfo zg::entities::PlaneFactory(
    glm::vec4 color,
    std::string name,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale,
    const shaders::RuntimeConstants constants,
	PlaneType planeType,
    zg::FRONTFACE frontFace
)
{
    auto mergedConstants = shaders::mergeConstants({
        shaders::RuntimeConstants({"Shape", "Color"}),
        shaders::common_zg_constants,
        constants
    });
    zg::MeshCreateInfo meshInfo{
        .name = "Plane",
        .shapeType = (ShapeType)planeType,
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
zg::EntityCreateInfo zg::entities::PlaneFactory(
    const std::shared_ptr<textures::Texture>& texture,
    std::string name,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale,
    const std::vector<glm::vec2>& uv2s,
    const shaders::RuntimeConstants constants,
	PlaneType planeType,
    zg::FRONTFACE frontFace
)
{
    auto mergedConstants = shaders::mergeConstants({
        shaders::RuntimeConstants({"Shape", "UV2", "ColorTexture"}),
        shaders::common_zg_constants,
        constants
    });
    zg::MeshCreateInfo meshInfo{
        .name = "Plane",
        .shapeType = (ShapeType)planeType,
        .material = {
            glm::vec4(1),
            1
        },
        .info = [uv2s](auto&) -> MeshInfo {
            return MeshInfo{ 
                .uv2s = uv2s
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