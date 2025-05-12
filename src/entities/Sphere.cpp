#include <zg/entities/Sphere.hpp>
#include <zg/Mesh.hpp>
#include <zg/utilities.hpp>
using namespace zg;
zg::EntityCreateInfo zg::entities::SphereFactory(glm::vec4 color, const std::string& name, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
    const shaders::RuntimeConstants& constants)
{
    zg::MeshCreateInfo meshInfo{
        .shapeType = ShapeType::Sphere,
        .material = [color](auto&) -> Material {
            return {
                color,
                0
            };
        },
        .constants = zg::mergeVectors<std::string>(
            {{"Shape", "Color", "SDFColor", "Position", "Normal", "View", "Projection", "Model", "CameraPosition"}}, constants)
    };
    zg::EntityCreateInfo info{
        .typeName = "Sphere",
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
// zg::EntityCreateInfo zg::entities::SphereFactory(const std::shared_ptr<textures::Texture>& texture, std::string name, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
//     const shaders::RuntimeConstants& constants, zg::FRONTFACE frontFace)
// {
//     zg::MeshCreateInfo meshInfo{
//         .shapeType = ShapeType::Sphere,
//         .material = [](auto&) -> Material {
//             return {
//                 glm::vec4(1),
//                 1
//             };
//         },
//         .uv2Count = [](auto&) { return 4; },
//         .uv2s = [](auto&) -> std::vector<glm::vec2>
//         {
//             return {
//                 {0, 0},
//                 {1, 0},
//                 {1, 1},
//                 {0, 1}
//             };
//         },
//         .keyedTextures = {
//             {"ColorTexture", texture}
//         },
//         .constants = zg::mergeVectors<std::string>(
// 			{{"Shape", "UV2", "Position", "Normal", "ColorTexture", "View", "Projection", "Model", "CameraPosition"}}, constants)
//     };
//     zg::EntityCreateInfo info{
//         .typeName = "Sphere",
//         .position = position,
//         .rotation = rotation,
//         .scale = scale,
//         .name = name,
//         .meshInfos = {
//             meshInfo
//         }
//     };
//     return info;
// }   