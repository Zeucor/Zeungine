#include <glm/fwd.hpp>
#include <stdexcept>
#include <zg/Logger.hpp>
#include <zg/Window.hpp>
#include <zg/lights/Lights.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/zgfilesystem/File.hpp>
#include <zg/zgfilesystem/Directory.hpp>
#include <zg/entities/SDF.hpp>
using namespace zg;
using namespace zg::shaders;
bool registered_zg_shader_hooks = false;
void zg::shaders::register_zg_shader_hooks()
{
  auto& sf = ShaderFactory::GetSingleton();
  sf.addHook(ShaderType::Vertex, "layout", "Color", [](auto& shader, const auto& constants)-> std::string
  {
    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
      return constant == "Shape";
    });
    if (found == constants.end())
    {
      auto& sf = ShaderFactory::GetSingleton();
      return "layout(location = " + std::to_string(sf.currentInLayoutIndex++) +
        ") in vec4 inColor;";
    }
    return "";
  });
  sf.addHook(ShaderType::Vertex, "layout", "Color", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    return "layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
      ") out vec4 outColor;";
  });
  sf.addHook(ShaderType::Vertex, "layout", "Position", [](auto& shader, const auto& constants)-> std::string
  {
    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
      return constant == "Shape";
    });
    if (found == constants.end())
    {
      auto& sf = ShaderFactory::GetSingleton();
      return "layout(location = " + std::to_string(sf.currentInLayoutIndex++) +
        ") in vec3 inPosition;";
    }
    return "";
  });
  sf.addHook(ShaderType::Vertex, "layout", "Position", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    return "layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
      ") out vec4 outPosition;";
  });
  sf.addHook(ShaderType::Vertex, "layout", "UV2", [](auto& shader, const auto& constants)-> std::string
  {
    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
      return constant == "Shape";
    });
    if (found == constants.end())
    {
      auto& sf = ShaderFactory::GetSingleton();
      return "layout(location = " + std::to_string(sf.currentInLayoutIndex++) +
        ") in vec2 inUV;";
    }
    return "";
  });
  sf.addHook(ShaderType::Vertex, "layout", "UV2", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    return "layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
      ") out vec2 outUV;";
  });
  sf.addHook(ShaderType::Vertex, "layout", "UV3", [](auto& shader, const auto& constants)-> std::string
  {
    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
      return constant == "Shape";
    });
    if (found == constants.end())
    {
      auto& sf = ShaderFactory::GetSingleton();
      return "layout(location = " + std::to_string(sf.currentInLayoutIndex++) +
        ") in vec3 inUV;";
    }
    return "";
  });
  sf.addHook(ShaderType::Vertex, "layout", "UV3", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    return "layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
      ") out vec3 outUV;";
  });
  sf.addHook(ShaderType::Vertex, "layout", "Viewport", [](auto& shader, const auto& constants) -> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addUBO(ShaderType::Vertex, "Viewport", bindingIndex, sizeof(glm::vec4));
    return "layout(binding = " + std::to_string(bindingIndex) + ") uniform ViewportBuffer {\n" +
      " vec4 size;\n" +
      "} Viewport;";
  });
  sf.addHook(ShaderType::Vertex, "layout", "NearFarPlanes", [](auto& shader, const auto& constants) -> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addUBO(ShaderType::Vertex, "NearFarPlanes", bindingIndex, sizeof(float) * 2);
    return "layout(binding = " + std::to_string(bindingIndex) + ") uniform NearFarPlaneBuffer {\n" +
      " float near;\n" +
      " float far;\n" +
      "} NearFarPlane;";
  });
  sf.addHook(ShaderType::Vertex, "layout", "Time", [](auto& shader, const auto& constants) -> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addUBO(ShaderType::Vertex, "Time", bindingIndex, sizeof(float));
    return "layout(binding = " + std::to_string(bindingIndex) + ") uniform TimeBuffer {\n" +
      " float seconds;\n" +
      "} Time;";
  });
  sf.addHook(ShaderType::Vertex, "layout", "Shape", [](auto& shader, const auto& constants) -> std::string
  {
    std::string string;
    auto& sf = ShaderFactory::GetSingleton();
    string += "layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
      ") out int outID;\n";
    string += R"(struct Entity {
  int shape_type;
  int material_index;
  int vertex_offset;
  int padding;
  int uv2_offset;
  int uv3_offset;
  int meta_int;
  float meta_float;
  vec4 meta_vec4;
};
const int SHAPE_TYPE_NONE = 0;
const int SHAPE_TYPE_BOX = 1;
const int SHAPE_TYPE_SDF = 2;
const int SHAPE_TYPE_MESH = 3;
const int SHAPE_TYPE_PLANE_XZ_CENTER = 4;
const int SHAPE_TYPE_PLANE_XY_CENTER = 5;
const int SHAPE_TYPE_PLANE_YZ_CENTER = 6;
const int SHAPE_TYPE_PLANE_XY_BOTTOM_LEFT = 7;
struct Material {
  vec4 albedo;
  int type; // 0 = albedo, 1 = uv2, 2 = uv3
};
Material get_material(in Entity entity);
Entity get_entity(int entity_id);
vec2 get_uv2(int vertex_id, in Entity entity, in Material material);
vec3 get_uv3(int vertex_id, in Entity entity, in Material material);
vec3 get_box_vertex(int vertex_id);
vec3 get_box_normal(int vertex_id);
vec4 get_box_color(int vertex_id, in Entity entity, in Material material);
vec3 get_box_uv3(int vertex_id, in Entity entity, in Material material);
vec3 get_plane_vertex_xz_center(int vertex_id);
vec3 get_plane_normal_xz(int vertex_id);
vec3 get_plane_normal_yz(int vertex_id);
vec3 get_plane_vertex_xy_center(int vertex_id);
vec3 get_plane_normal_xy(int vertex_id);
vec3 get_plane_normal_yz(int vertex_id);
vec4 get_plane_color(int vertex_id, in Entity entity, in Material material);
vec2 get_plane_uv2(int vertex_id, in Entity entity, in Material material);
vec3 get_mesh_vertex(int vertex_id, in Entity entity, in Material material);
vec3 get_mesh_normal(int vertex_id, in Entity entity, in Material material);
vec4 get_mesh_color(int vertex_id, in Entity entity, in Material material);
vec2 get_mesh_uv2(int vertex_id, in Entity entity, in Material material);
int get_entity_id();
vec3 get_entity_vertex(int vertex_id, in Entity entity, in Material material);
vec3 get_entity_normal(int vertex_id, in Entity entity, in Material material);
vec4 get_entity_color(int vertex_id, in Entity entity, in Material material);
vec2 get_entity_uv2(int vertex_id, in Entity entity, in Material material);
vec3 get_entity_uv3(int vertex_id, in Entity entity, in Material material);
)";
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Vertex, "Entities", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer EntitiesBuffer {\n" +
        "    Entity data[];\n" +
        "} Entities;\n";
    bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Vertex, "Materials", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer MaterialsBuffer {\n" +
        "    Material data[];\n" +
        "} Materials;\n";
    bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Vertex, "MeshPositions", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer MeshPositionsBuffer {\n" +
        "    vec4 data[];\n" +
        "} MeshPositions;\n";
    // bindingIndex = sf.currentBindingIndex++;
    // shader.addSSBO(ShaderType::Vertex, "MeshNormals", bindingIndex);
    // string += "layout(std430, binding = " + std::to_string(bindingIndex) +
    //     ") buffer MeshNormalsBuffer {\n" +
    //     "    vec4 data[];\n" +
    //     "} MeshNormals;\n";
    bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Vertex, "EntityUV2s", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer EntityUV2sBuffer {\n" +
        "    vec2 data[];\n" +
        "} EntityUV2s;\n";
    bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Vertex, "EntityUV3s", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer EntityUV3sBuffer {\n" +
        "    vec3 data[];\n" +
        "} EntityUV3s;\n";
      return string;
  });
  sf.addHook(ShaderType::Vertex, "layout", "View", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Vertex, "InverseInstanceViews", bindingIndex);
    std::string string;
    string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer InverseInstanceViewsBuffer {\n" +
        "    mat4 data[];\n" +
        "} InverseInstanceViews;\n";
    bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Vertex, "InstanceViews", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer InstanceViewsBuffer {\n" +
        "    mat4 data[];\n" +
        "} InstanceViews;\n";
    return string;
  });
  sf.addHook(ShaderType::Vertex, "layout", "Projection", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Vertex, "InverseInstanceProjections", bindingIndex);
    std::string string;
    string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer InverseInstanceProjectionsBuffer {\n" +
        "    mat4 data[];\n" +
        "} InverseInstanceProjections;\n";
    bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Vertex, "InstanceProjections", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer InstanceProjectionsBuffer {\n" +
        "    mat4 data[];\n" +
        "} InstanceProjections;\n";
    return string;
  });
  sf.addHook(ShaderType::Vertex, "layout", "Model", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Vertex, "InverseInstanceModels", bindingIndex);
    std::string string;
    string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer InverseInstanceModelsBuffer {\n" +
        "    mat4 data[];\n" +
        "} InverseInstanceModels;\n";
    bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Vertex, "InstanceModels", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer InstanceModelsBuffer {\n" +
        "    mat4 data[];\n" +
        "} InstanceModels;\n";
    return string;
  });
  sf.addHook(ShaderType::Vertex, "layout", "Normal", [](auto& shader, const auto& constants)-> std::string
  {
    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
      return constant == "Shape";
    });
    if (found == constants.end())
    {
      auto& sf = ShaderFactory::GetSingleton();
      return "layout(location = " + std::to_string(sf.currentInLayoutIndex++) +
        ") in vec3 inNormal;\n";
    }
    return "";
  });
  sf.addHook(ShaderType::Vertex, "layout", "Normal", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto string = std::string(
      "layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
      ") out vec4 outFragPosition;\n");
    string += "layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
      ") out vec3 outNormal;";
    return string;
  });
  sf.addHook(ShaderType::Vertex, "layout", "LightSpacePosition", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    std::string string;
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addUBO(ShaderType::Vertex, "DirectionalLightSpaceMatrices", bindingIndex, sizeof(glm::mat4));
    string += "layout(binding = " + std::to_string(bindingIndex) +
      ") uniform DirectionalLightSpaceMatrices {\n" +
      " mat4 matrix[1];\n" +
      "} directionalLightSpaceMatrices;\n";
    string += "layout(location = " + std::to_string(sf.currentOutLayoutIndex) +
      ") out vec4 outDirectionalLightSpaceVertices[1];\n";
    sf.currentOutLayoutIndex += 1;
    bindingIndex = sf.currentBindingIndex++;
    shader.addUBO(ShaderType::Vertex, "SpotLightSpaceMatrices", bindingIndex, sizeof(glm::mat4));
    string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform SpotLightSpaceMatrices {\n"
      +
      "  mat4 matrix[1];\n" +
      "} spotLightSpaceMatrices;\n";
    string += "layout(location = " + std::to_string(sf.currentOutLayoutIndex) +
      ") out vec4 outSpotLightSpaceVertices[1];";
    sf.currentOutLayoutIndex += 1;
    return string;
  });
  sf.addHook(ShaderType::Vertex, "preInMain", "Shape", [](auto& shader, const auto& constants) -> std::string
  {
    return R"(  int entity_id = get_entity_id();
  outID = entity_id;
  Entity entity = get_entity(entity_id);
  Material material = get_material(entity);)";
  });
  sf.addHook(ShaderType::Vertex, "preInMain", "Color", [](auto& shader, const auto& constants)-> std::string
  {
    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
      return constant == "Shape";
    });
    if (found == constants.end())
    {
      return "  outColor = inColor;";
    }
    return R"(  outColor = get_entity_color(gl_VertexIndex, entity, material);)";
  });
  sf.addHook(ShaderType::Vertex, "preInMain", "Position", [](auto& shader, const RuntimeConstants& constants)-> std::string
  {
    bool hasSkyBox = std::find(constants.begin(), constants.end(), "SkyBox") != constants.end();
    std::string string;
    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
      return constant == "Shape";
    });
    if (found != constants.end())
    {
      string += "  vec3 inPosition = get_entity_vertex(gl_VertexIndex, entity, material);\n";
    }
    if (hasSkyBox)
    {
      string += "  vec4 pos = ";
    }
    else
    {
      string += "  gl_Position = ";
    }
    // Projection / View not run when LightSpaceMatrix
    if (std::find(constants.begin(), constants.end(), "Projection") != constants.end())
    {
      string += "InstanceProjections.data[gl_InstanceIndex] * ";
    }
    if (std::find(constants.begin(), constants.end(), "View") != constants.end())
    {
      string += "InstanceViews.data[gl_InstanceIndex] * ";
    }
    if (std::find(constants.begin(), constants.end(), "Model") != constants.end())
    {
      string += "InstanceModels.data[gl_InstanceIndex] * ";
    }
    string += "vec4(inPosition, 1);\n";
    if (hasSkyBox)
    {
      string += "  gl_Position = pos.xyww;\n";
    }
    string += "  outPosition = gl_Position;";
    return string;
  });
  sf.addHook(ShaderType::Vertex, "preInMain", "LightSpacePosition", [](auto& shader, const RuntimeConstants& constants)-> std::string
  {
    std::string string("  for (int i = 0; i < 4; ++i){\n");
    string += "    outDirectionalLightSpaceVertices[i] = ";
    string += "directionalLightSpaceMatrices.matrix[i] * ";
    if (std::find(constants.begin(), constants.end(), "Model") != constants.end())
    {
      string += "InstanceModels.data[gl_InstanceIndex] * ";
    }
    string += "vec4(inPosition, 1);\n";
    string += "    outSpotLightSpaceVertices[i] = ";
    string += "spotLightSpaceMatrices.matrix[i] * ";
    if (std::find(constants.begin(), constants.end(), "Model") != constants.end())
    {
      string += "InstanceModels.data[gl_InstanceIndex] * ";
    }
    string += "vec4(inPosition, 1);\n";
    string += "  }";
    return string;
  });
  sf.addHook(ShaderType::Vertex, "preInMain", "Normal", [](auto& shader, const auto& constants)-> std::string
  {
    std::string string;
    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
      return constant == "Shape";
    });
    if (found != constants.end())
    {
      string += "  vec3 inNormal = get_entity_normal(gl_VertexIndex, entity, material);\n";
    }
    string += "  outFragPosition = InstanceModels.data[gl_InstanceIndex] * vec4(inPosition, 1.0);\n";
    string += "  outNormal = mat3(transpose(inverse(InstanceModels.data[gl_InstanceIndex]))) * inNormal;";
    return string;
  });
  sf.addHook(ShaderType::Vertex, "preInMain", "UV2", [](auto& shader, const auto& constants)-> std::string
  {
    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
      return constant == "Shape";
    });
    if (found != constants.end())
    {
      return "  outUV = get_entity_uv2(gl_VertexIndex, entity, material);";
    }
    return "  outUV = inUV;";
  });
  sf.addHook(ShaderType::Vertex, "preInMain", "UV3", [](auto& shader, const auto& constants)-> std::string
  {
    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
      return constant == "Shape";
    });
    if (found != constants.end())
    {
      return "  outUV = normalize(get_entity_uv3(gl_VertexIndex, entity, material));";
    }
    return "  outUV = inUV;";
  });
  sf.addHook(ShaderType::Vertex, "postMain", "Shape", [](auto& shader, const auto& constants) -> std::string
  {
    std::string string;
    string += R"(
Material get_material(in Entity entity)
{
  int material_index = entity.material_index;
  if (material_index >= 0) {
    return Materials.data[material_index];
  }
  Material defaultMaterial;
  defaultMaterial.type = 0;
  defaultMaterial.albedo = vec4(1.0);
  return defaultMaterial;
}
Entity get_entity(int entity_id)
{
  if (entity_id >= 0) {
    return Entities.data[entity_id];
  }
  Entity defaultEntity;
  defaultEntity.shape_type = -1;
  defaultEntity.material_index = -1;
  defaultEntity.vertex_offset = -1;
  defaultEntity.uv2_offset = -1;
  defaultEntity.uv3_offset = -1;
  return defaultEntity;
}
vec4 get_color(int vertex_id, in Entity entity, in Material material)
{
  // if (material.type != 0)
  //   return vec4(1, 1, 1, 1);
  return material.albedo;
  // return vec4(material.type, material.type, material.type, 1);
}
vec2 get_box_uv2(int vertex_id)
{
    // These UV coordinates correspond to the 8 unique corners (v0-v7)
    // as defined in get_box_vertex, projected onto a 2D face.
    // Each face covers the UV range from (0,0) to (1,1).

    switch (vertex_id)
    {
        // Front face (-Z) - Triangles: (v0,v1,v2), (v1,v3,v2)
        // Mapping (X, Y) to (U, V)
        case 0:  return vec2(0.0, 0.0); // Corresponds to v0 (-0.5, -0.5, -0.5) -> bottom-left
        case 1:  return vec2(1.0, 0.0); // Corresponds to v1 ( 0.5, -0.5, -0.5) -> bottom-right
        case 2:  return vec2(0.0, 1.0); // Corresponds to v2 (-0.5,  0.5, -0.5) -> top-left
        case 3:  return vec2(1.0, 0.0); // Corresponds to v1 ( 0.5, -0.5, -0.5) -> bottom-right
        case 4:  return vec2(1.0, 1.0); // Corresponds to v3 ( 0.5,  0.5, -0.5) -> top-right
        case 5:  return vec2(0.0, 1.0); // Corresponds to v2 (-0.5,  0.5, -0.5) -> top-left

        // Back face (+Z) - Triangles: (v5,v4,v7), (v4,v6,v7)
        // Mapping (-X, Y) to (U, V) for consistent front-facing texture.
        // Or if looking from outside, X goes from 0.5 to -0.5, Y from -0.5 to 0.5
        // (1-x) for U mapping X from 0.5 to -0.5 to 0 to 1
        case 6:  return vec2(0.0, 0.0); // Corresponds to v5 ( 0.5, -0.5,  0.5) -> relative bottom-left on texture if Z+ is "front"
        case 7:  return vec2(1.0, 0.0); // Corresponds to v4 (-0.5, -0.5,  0.5) -> relative bottom-right
        case 8:  return vec2(0.0, 1.0); // Corresponds to v7 ( 0.5,  0.5,  0.5) -> relative top-left
        case 9:  return vec2(1.0, 0.0); // Corresponds to v4 (-0.5, -0.5,  0.5) -> relative bottom-right
        case 10: return vec2(1.0, 1.0); // Corresponds to v6 (-0.5,  0.5,  0.5) -> relative top-right
        case 11: return vec2(0.0, 1.0); // Corresponds to v7 ( 0.5,  0.5,  0.5) -> relative top-left

        // Left face (-X) - Triangles: (v4,v0,v6), (v0,v2,v6)
        // Mapping (Z, Y) to (U, V)
        case 12: return vec2(0.0, 0.0); // Corresponds to v4 (-0.5, -0.5,  0.5) -> bottom-left
        case 13: return vec2(1.0, 0.0); // Corresponds to v0 (-0.5, -0.5, -0.5) -> bottom-right
        case 14: return vec2(0.0, 1.0); // Corresponds to v6 (-0.5,  0.5,  0.5) -> top-left
        case 15: return vec2(1.0, 0.0); // Corresponds to v0 (-0.5, -0.5, -0.5) -> bottom-right
        case 16: return vec2(1.0, 1.0); // Corresponds to v2 (-0.5,  0.5, -0.5) -> top-right
        case 17: return vec2(0.0, 1.0); // Corresponds to v6 (-0.5,  0.5,  0.5) -> top-left

        // Right face (+X) - Triangles: (v1,v5,v3), (v5,v7,v3)
        // Mapping (-Z, Y) to (U, V) for consistent front-facing texture.
        case 18: return vec2(0.0, 0.0); // Corresponds to v1 ( 0.5, -0.5, -0.5) -> bottom-left
        case 19: return vec2(1.0, 0.0); // Corresponds to v5 ( 0.5, -0.5,  0.5) -> bottom-right
        case 20: return vec2(0.0, 1.0); // Corresponds to v3 ( 0.5,  0.5, -0.5) -> top-left
        case 21: return vec2(1.0, 0.0); // Corresponds to v5 ( 0.5, -0.5,  0.5) -> bottom-right
        case 22: return vec2(1.0, 1.0); // Corresponds to v7 ( 0.5,  0.5,  0.5) -> top-right
        case 23: return vec2(0.0, 1.0); // Corresponds to v3 ( 0.5,  0.5, -0.5) -> top-left

        // Top face (+Y) - Triangles: (v2,v3,v6), (v3,v7,v6)
        // Mapping (X, -Z) to (U, V) assuming standard top-down view (positive Z is "down" on texture)
        case 24: return vec2(0.0, 0.0); // Corresponds to v2 (-0.5,  0.5, -0.5) -> bottom-left (relative to texture)
        case 25: return vec2(1.0, 0.0); // Corresponds to v3 ( 0.5,  0.5, -0.5) -> bottom-right
        case 26: return vec2(0.0, 1.0); // Corresponds to v6 (-0.5,  0.5,  0.5) -> top-left
        case 27: return vec2(1.0, 0.0); // Corresponds to v3 ( 0.5,  0.5, -0.5) -> bottom-right
        case 28: return vec2(1.0, 1.0); // Corresponds to v7 ( 0.5,  0.5,  0.5) -> top-right
        case 29: return vec2(0.0, 1.0); // Corresponds to v6 (-0.5,  0.5,  0.5) -> top-left

        // Bottom face (-Y) - Triangles: (v4,v5,v0), (v5,v1,v0)
        // Mapping (X, Z) to (U, V) assuming standard bottom-up view
        case 30: return vec2(0.0, 0.0); // Corresponds to v4 (-0.5, -0.5,  0.5) -> bottom-left
        case 31: return vec2(1.0, 0.0); // Corresponds to v5 ( 0.5, -0.5,  0.5) -> bottom-right
        case 32: return vec2(0.0, 1.0); // Corresponds to v0 (-0.5, -0.5, -0.5) -> top-left
        case 33: return vec2(1.0, 0.0); // Corresponds to v5 ( 0.5, -0.5,  0.5) -> bottom-right
        case 34: return vec2(1.0, 1.0); // Corresponds to v1 ( 0.5, -0.5, -0.5) -> top-right
        case 35: return vec2(0.0, 1.0); // Corresponds to v0 (-0.5, -0.5, -0.5) -> top-left

        default: return vec2(0.0, 0.0);
    }
}

vec2 get_plane_uv2_xz_center(int vertex_id)
{
    switch (vertex_id)
    {
        case 0: return vec2(0.0, 0.0); // Corresponds to (-0.5, -0.5) in XZ
        case 1: return vec2(1.0, 0.0); // Corresponds to ( 0.5, -0.5) in XZ
        case 2: return vec2(0.0, 1.0); // Corresponds to (-0.5,  0.5) in XZ
        case 3: return vec2(0.0, 1.0); // Corresponds to (-0.5,  0.5) in XZ
        case 4: return vec2(1.0, 0.0); // Corresponds to ( 0.5, -0.5) in XZ
        case 5: return vec2(1.0, 1.0); // Corresponds to ( 0.5,  0.5) in XZ
        default: return vec2(0.0, 0.0);
    }
}

vec2 get_plane_uv2_xy_center(int vertex_id)
{
    switch (vertex_id)
    {
        case 2: return vec2(0.0, 1.0); // Corresponds to (-0.5, -0.5) in XY
        case 1: return vec2(1.0, 1.0); // Corresponds to ( 0.5, -0.5) in XY
        case 0: return vec2(0.0, 0.0); // Corresponds to (-0.5,  0.5) in XY
        case 5: return vec2(0.0, 0.0); // Corresponds to (-0.5,  0.5) in XY
        case 4: return vec2(1.0, 1.0); // Corresponds to ( 0.5, -0.5) in XY
        case 3: return vec2(1.0, 0.0); // Corresponds to ( 0.5,  0.5) in XY
        default: return vec2(0.0, 0.0);
    }
}

vec2 get_plane_uv2_xy_bottom_left(int vertex_id)
{
    switch (vertex_id)
    {
        case 0: return vec2(0.0, 1.0);
        case 1: return vec2(1.0, 0.0);
        case 2: return vec2(0.0, 0.0);
        case 3: return vec2(1.0, 1.0);
        case 4: return vec2(1.0, 0.0);
        case 5: return vec2(0.0, 1.0);
      default: return vec2(0.0, 0.0);
    }
}

vec2 get_plane_uv2_yz_center(int vertex_id)
{
    switch (vertex_id)
    {
        case 0: return vec2(0.0, 0.0); // Corresponds to (-0.5, -0.5) in YZ (Y as U, Z as V)
        case 1: return vec2(1.0, 0.0); // Corresponds to ( 0.5, -0.5) in YZ
        case 2: return vec2(0.0, 1.0); // Corresponds to (-0.5,  0.5) in YZ
        case 3: return vec2(0.0, 1.0); // Corresponds to (-0.5,  0.5) in YZ
        case 4: return vec2(1.0, 0.0); // Corresponds to ( 0.5, -0.5) in YZ
        case 5: return vec2(1.0, 1.0); // Corresponds to ( 0.5,  0.5) in YZ
        default: return vec2(0.0, 0.0);
    }
}
vec2 get_mesh_uv2(int vertex_id, in Entity entity, in Material material)
{
  return EntityUV2s.data[entity.uv2_offset + vertex_id];
}
vec2 get_uv2(int vertex_id, in Entity entity, in Material material)
{
  if (entity.uv2_offset != -1)
    return get_mesh_uv2(vertex_id, entity, material);
  switch (entity.shape_type)
  {
  case SHAPE_TYPE_BOX:
    return get_box_uv2(vertex_id);
  case SHAPE_TYPE_PLANE_XZ_CENTER:
    return get_plane_uv2_xz_center(vertex_id);
  case SHAPE_TYPE_PLANE_XY_CENTER:
    return get_plane_uv2_xy_center(vertex_id);
  case SHAPE_TYPE_PLANE_XY_BOTTOM_LEFT:
    return get_plane_uv2_xy_bottom_left(vertex_id);
  case SHAPE_TYPE_PLANE_YZ_CENTER:
    return get_plane_uv2_yz_center(vertex_id);
  case SHAPE_TYPE_SDF:
  {
    return vec2(0.0);
  }
  case SHAPE_TYPE_MESH:
    return get_mesh_uv2(vertex_id, entity, material);
  }
  return vec2(0.0);
}
vec3 get_uv3(int vertex_id, in Entity entity, in Material material)
{
  if (material.type != 2)
    return vec3(0, 0, 0);
  return EntityUV3s.data[entity.uv3_offset + vertex_id];
}
vec3 get_box_vertex(int vertex_id)
{
  // Define the 8 unique corners of the cube
  vec3 v0 = vec3(-0.5, -0.5, -0.5); // NLL
  vec3 v1 = vec3( 0.5, -0.5, -0.5); // NLR
  vec3 v2 = vec3(-0.5,  0.5, -0.5); // NUL
  vec3 v3 = vec3( 0.5,  0.5, -0.5); // NUR
  vec3 v4 = vec3(-0.5, -0.5,  0.5); // FLL
  vec3 v5 = vec3( 0.5, -0.5,  0.5); // FLR
  vec3 v6 = vec3(-0.5,  0.5,  0.5); // FUL
  vec3 v7 = vec3( 0.5,  0.5,  0.5); // FUR

  switch (vertex_id)
  {
    // Front face (-Z) - Triangles: (v0,v1,v2), (v1,v3,v2)
    case 0:  return v0; // NLL
    case 1:  return v1; // NLR
    case 2:  return v2; // NUL
    case 3:  return v1; // NLR
    case 4:  return v3; // NUR
    case 5:  return v2; // NUL

    // Back face (+Z) - Triangles: (v5,v4,v7), (v4,v6,v7)
    case 6:  return v5; // FLR
    case 7:  return v4; // FLL
    case 8:  return v7; // FUR
    case 9:  return v4; // FLL
    case 10: return v6; // FUL
    case 11: return v7; // FUR

    // Left face (-X) - Triangles: (v4,v0,v6), (v0,v2,v6)
    case 12: return v4; // FLL
    case 13: return v0; // NLL
    case 14: return v6; // FUL
    case 15: return v0; // NLL
    case 16: return v2; // NUL
    case 17: return v6; // FUL

    // Right face (+X) - Triangles: (v1,v5,v3), (v5,v7,v3)
    case 18: return v1; // NLR
    case 19: return v5; // FLR
    case 20: return v3; // NUR
    case 21: return v5; // FLR
    case 22: return v7; // FUR
    case 23: return v3; // NUR

    // Top face (+Y) - Triangles: (v2,v3,v6), (v3,v7,v6)
    case 24: return v2; // NUL
    case 25: return v3; // NUR
    case 26: return v6; // FUL
    case 27: return v3; // NUR
    case 28: return v7; // FUR
    case 29: return v6; // FUL

    // Bottom face (-Y) - Triangles: (v4,v5,v0), (v5,v1,v0)
    case 30: return v4; // FLL
    case 31: return v5; // FLR
    case 32: return v0; // NLL
    case 33: return v5; // FLR
    case 34: return v1; // NLR
    case 35: return v0; // NLL

    default: return vec3(0.0, 0.0, 0.0); // Should not happen if vertexCount is 36
  }
}
vec3 get_box_normal(int vertex_id)
{
  // Normals for each face of the cube
  // Each face has 6 vertices (2 triangles), and all 6 share the same normal.

  // Front face (-Z normal)
  if (vertex_id >= 0 && vertex_id <= 5) {
    return vec3(0.0, 0.0, -1.0);
  }
  // Back face (+Z normal)
  else if (vertex_id >= 6 && vertex_id <= 11) {
    return vec3(0.0, 0.0, 1.0);
  }
  // Left face (-X normal)
  else if (vertex_id >= 12 && vertex_id <= 17) {
    return vec3(-1.0, 0.0, 0.0);
  }
  // Right face (+X normal)
  else if (vertex_id >= 18 && vertex_id <= 23) {
    return vec3(1.0, 0.0, 0.0);
  }
  // Top face (+Y normal)
  else if (vertex_id >= 24 && vertex_id <= 29) {
    return vec3(0.0, 1.0, 0.0);
  }
  // Bottom face (-Y normal)
  else if (vertex_id >= 30 && vertex_id <= 35) {
    return vec3(0.0, -1.0, 0.0);
  }
  
  // Default fallback, though with correct vertex_id (0-35) this shouldn't be reached.
  return vec3(0.0, 0.0, 0.0); 
}
vec3 get_box_uv3(int vertex_id, in Entity entity, in Material material)
{
  if (material.type == 2)
  {
    return get_uv3(vertex_id, entity, material);
  }
  return vec3(0, 0, 0);
}
vec3 get_plane_vertex_xz_center(int vertex_id)
{
  switch (vertex_id)
  {
    case 0: return vec3(-0.5, 0.0, -0.5);
    case 1: return vec3( 0.5, 0.0, -0.5);
    case 2: return vec3(-0.5, 0.0,  0.5);
    case 3: return vec3(-0.5, 0.0,  0.5);
    case 4: return vec3( 0.5, 0.0, -0.5);
    case 5: return vec3( 0.5, 0.0,  0.5);
    default: return vec3(0.0, 0.0, 0.0);
  }
}
vec3 get_plane_vertex_xy_center(int vertex_id)
{
  switch (vertex_id)
  {
    case 2: return vec3(-0.5, -0.5, 0.0);
    case 1: return vec3( 0.5, -0.5, 0.0);
    case 0: return vec3(-0.5,  0.5, 0.0);
    case 5: return vec3(-0.5,  0.5, 0.0);
    case 4: return vec3( 0.5, -0.5, 0.0);
    case 3: return vec3( 0.5,  0.5, 0.0);
    default: return vec3(0.0, 0.0, 0.0);
  }
}
vec3 get_plane_vertex_yz_center(int vertex_id)
{
  switch (vertex_id)
  {
    case 0: return vec3( 0.0, -0.5, -0.5);
    case 1: return vec3( 0.0, -0.5, 0.5);
    case 2: return vec3( 0.0,  0.5, -0.5);
    case 3: return vec3( 0.0,  0.5, -0.5);
    case 4: return vec3( 0.0, -0.5, 0.5);
    case 5: return vec3( 0.0,  0.5, 0.5);
    default: return vec3(0.0, 0.0, 0.0);
  }
}
vec3 get_plane_vertex_xy_bottom_left(int vertex_id)
{
  switch (vertex_id)
  {
    case 0: return vec3(0.0, 1.0, 0.0);
    case 1: return vec3(1.0, 0.0, 0.0);
    case 2: return vec3(0.0, 0.0, 0.0);
    case 3: return vec3(1.0, 1.0, 0.0);
    case 4: return vec3(1.0, 0.0, 0.0);
    case 5: return vec3(0.0, 1.0, 0.0);
   default: return vec3(0.0, 0.0, 0.0);
  }
}
vec3 get_plane_normal_xy(int vertex_id)
{
  return vec3(0, 0, 1);
}
vec3 get_plane_normal_xz(int vertex_id)
{
  return vec3(0, 1, 0);
}
vec3 get_plane_normal_yz(int vertex_id)
{
  return vec3(1, 0, 0);
}
vec2 get_plane_uv2(int vertex_id, in Entity entity, in Material material)
{
  if (material.type == 2)
  {
    return get_uv2(vertex_id, entity, material);
  }
  return vec2(0, 0);
}
vec3 get_mesh_vertex(int vertex_id, in Entity entity, in Material material)
{
  return MeshPositions.data[entity.vertex_offset + vertex_id].xyz;
}
vec3 get_mesh_normal(int vertex_id, in Entity entity, in Material material)
{
  int triangle_base_vertex_id = (vertex_id / 3) * 3;
  int v0_global_index = entity.vertex_offset + triangle_base_vertex_id + 2;
  int v1_global_index = entity.vertex_offset + triangle_base_vertex_id + 1;
  int v2_global_index = entity.vertex_offset + triangle_base_vertex_id + 0;
  vec3 pos0 = MeshPositions.data[v0_global_index].xyz;
  vec3 pos1 = MeshPositions.data[v1_global_index].xyz;
  vec3 pos2 = MeshPositions.data[v2_global_index].xyz;
  vec3 edge1 = pos1 - pos0;
  vec3 edge2 = pos2 - pos0;
  vec3 normal = normalize(cross(edge1, edge2));
  return normal;
}
int get_entity_id()
{
  return gl_InstanceIndex;
};
vec3 get_entity_vertex(int vertex_id, in Entity entity, in Material material)
{
  switch (entity.shape_type)
  {
  case SHAPE_TYPE_BOX:
    return get_box_vertex(vertex_id);
  case SHAPE_TYPE_PLANE_XZ_CENTER:
    return get_plane_vertex_xz_center(vertex_id);
  case SHAPE_TYPE_PLANE_XY_CENTER:
    return get_plane_vertex_xy_center(vertex_id);
  case SHAPE_TYPE_PLANE_YZ_CENTER:
    return get_plane_vertex_yz_center(vertex_id);
  case SHAPE_TYPE_PLANE_XY_BOTTOM_LEFT:
    return get_plane_vertex_xy_bottom_left(vertex_id);
  case SHAPE_TYPE_SDF:
  {
    // vec3 plane_vertex = get_plane_vertex_xy_center(vertex_id) * 5.0; // Local quad in XY plane
    // mat4 modelMatrix = InstanceModels.data[gl_InstanceIndex];
    // mat4 inverseModelMatrix = InverseInstanceModels.data[gl_InstanceIndex];
    // mat4 viewMatrix = InstanceViews.data[gl_InstanceIndex];
    // mat4 inverseViewMatrix = InverseInstanceViews.data[gl_InstanceIndex];
    // mat4 invModelView = inverseViewMatrix * inverseModelMatrix; // Inverse Model-View
    // vec3 entity_center_view = (viewMatrix * modelMatrix * vec4(0.0, 0.0, 0.0, 1.0)).xyz;
    // vec3 desired_view_pos = entity_center_view + vec3(plane_vertex.xy, 0.0); // Add offset in view space
    // vec4 desired_world_pos_homogenous = inverseViewMatrix * vec4(desired_view_pos, 1.0);
    // vec3 desired_world_pos = desired_world_pos_homogenous.xyz / desired_world_pos_homogenous.w;
    // vec4 inPosition_homogenous = inverseModelMatrix * vec4(desired_world_pos, 1.0);
    // return inPosition_homogenous.xyz / inPosition_homogenous.w;
    vec3 vertex_local = get_plane_vertex_xy_center(vertex_id) * 1.0;
    // float scaleZ = length(InstanceModels.data[gl_InstanceIndex][2].xyz);
    vertex_local.z = 0.5;
)";
    auto constants_end = constants.end();
    if (std::find(constants.begin(), constants_end, "View") == constants_end)
    {
      string += R"(  return vertex_local;
)";
    }
    else
    {
      string += R"(
    mat4 viewMatrix = InstanceViews.data[gl_InstanceIndex];
    mat3 viewRotationInverse = transpose(mat3(viewMatrix));
    vec3 vertex_rot = viewRotationInverse * vertex_local;
    return vertex_rot;
)";
    }
  string += R"(
  }
  case SHAPE_TYPE_MESH:
    return get_mesh_vertex(vertex_id, entity, material);
  }
  return vec3(0.0);
}
vec3 get_entity_normal(int vertex_id, in Entity entity, in Material material)
{
  switch (entity.shape_type)
  {
  case SHAPE_TYPE_BOX:
    return get_box_normal(vertex_id);
  case SHAPE_TYPE_PLANE_XZ_CENTER:
    return get_plane_normal_xz(vertex_id);
  case SHAPE_TYPE_PLANE_XY_CENTER:
    return get_plane_normal_xy(vertex_id);
  case SHAPE_TYPE_PLANE_YZ_CENTER:
    return get_plane_normal_yz(vertex_id);
  case SHAPE_TYPE_SDF:
    return get_plane_normal_xy(vertex_id);
  case SHAPE_TYPE_MESH:
    return get_mesh_normal(vertex_id, entity, material);
  }
  return vec3(0.0);
}
vec4 get_entity_color(int vertex_id, in Entity entity, in Material material)
{
  return get_color(vertex_id, entity, material);
}
vec2 get_entity_uv2(int vertex_id, in Entity entity, in Material material)
{
  return get_uv2(vertex_id, entity, material);
}
vec3 get_entity_uv3(int vertex_id, in Entity entity, in Material material)
{
  return get_uv3(vertex_id, entity, material);
}
)";
    return string;
  });
  sf.addHook(ShaderType::Geometry, "preLayout", "Position", [](auto &shader, const auto &constants)->std::string
  {
    return std::string("layout(triangles) in;\n") +
      "layout(triangle_strip, max_vertices = 18) out;";
  });
  sf.addHook(ShaderType::Geometry, "layout", "PointLightSpaceMatrix", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addUBO(ShaderType::Geometry, "PointLightSpaceMatrix", bindingIndex, sizeof(glm::mat4));
    return "layout(binding = " + std::to_string(bindingIndex) + ") uniform PointLightSpaceMatrix {\n" +
      "  mat4 matrix[6];\n" +
      "} pointLightSpaceMatrix;";
  });
  sf.addHook(ShaderType::Geometry, "layout", "Position", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    return "layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
      ") out vec4 outFragPosition;";
  });
  sf.addHook(ShaderType::Geometry, "preInMain", "PointLightSpaceMatrix", [](auto &shader, const auto &constants)->std::string
  {
    return std::string("  for (int face = 0; face < 6; ++face){\n") +
      "    gl_Layer = face;\n" +
      "    for (int i = 0; i < 3; ++i){\n" +
      "      outFragPosition = gl_in[i].gl_Position;\n" +
      "      gl_Position = pointLightSpaceMatrix.matrix[face] * outFragPosition;\n" +
      "      EmitVertex();\n" +
      "    }\n" +
      "    EndPrimitive();\n" +
      "  }";
  });
  sf.addHook(ShaderType::Fragment, "layout", "Viewport", [](auto& shader, const auto& constants) -> std::string
  {
    auto bindingIndex = shader.getUBO_BindingIndex("Viewport");
    shader.addUBO(ShaderType::Fragment, "Viewport", bindingIndex, sizeof(glm::vec4));
    return "layout(binding = " + std::to_string(bindingIndex) + ") uniform ViewportBuffer {\n" +
      " vec4 size;\n" +
      "} Viewport;";
  });
  sf.addHook(ShaderType::Fragment, "layout", "NearFarPlanes", [](auto& shader, const auto& constants) -> std::string
  {
    auto bindingIndex = shader.getUBO_BindingIndex("NearFarPlanes");
    shader.addUBO(ShaderType::Fragment, "NearFarPlanes", bindingIndex, sizeof(float) * 2);
    return "layout(binding = " + std::to_string(bindingIndex) + ") uniform NearFarPlaneBuffer {\n" +
      " float near;\n" +
      " float far;\n" +
      "} NearFarPlane;";
  });
  sf.addHook(ShaderType::Fragment, "layout", "Time", [](auto& shader, const auto& constants) -> std::string
  {
    auto bindingIndex = shader.getUBO_BindingIndex("Time");
    shader.addUBO(ShaderType::Fragment, "Time", bindingIndex, sizeof(float));
    return "layout(binding = " + std::to_string(bindingIndex) + ") uniform TimeBuffer {\n" +
      " float seconds;\n" +
      "} Time;";
  });
  sf.addHook(ShaderType::Fragment, "layout", "Shape", [](auto& shader, const auto& constants) -> std::string
  {
    std::string string;
    auto& sf = ShaderFactory::GetSingleton();
    string += "layout(location = " + std::to_string(sf.currentInLayoutIndex++) +
      ") flat in int inID;\n";
    string += R"(struct Entity {
  int shape_type;
  int material_index;
  int vertex_offset;
  int padding;
  int uv2_offset;
  int uv3_offset;
  int meta_int;
  int meta_float;
  vec4 meta_vec4;
};
const int SHAPE_TYPE_NONE = 0;
const int SHAPE_TYPE_BOX = 1;
const int SHAPE_TYPE_SDF = 2;
const int SHAPE_TYPE_MESH = 3;
const int SHAPE_TYPE_PLANE_XZ_CENTER = 4;
const int SHAPE_TYPE_PLANE_XY_CENTER = 5;
const int SHAPE_TYPE_PLANE_YZ_CENTER = 6;
const int SHAPE_TYPE_PLANE_XY_BOTTOM_LEFT = 7;
struct Material {
  vec4 albedo;
  int type; // 0 = albedo, 1 = uv2, 2 = uv3
};
Material get_material(in Entity entity);
Entity get_entity(int entity_id);
vec4 get_box_color(int vertex_id, in Entity entity, in Material material);
vec4 get_plane_color(int vertex_id, in Entity entity, in Material material);
vec4 get_mesh_color(int vertex_id, in Entity entity, in Material material);
int get_entity_id();
vec4 get_entity_color(in Entity entity, in Material material);
vec3 get_entity_normal(in Entity entity, in Material material);
)";
    auto bindingIndex = shader.getSSBO_BindingIndex("Entities");
    shader.addSSBO(ShaderType::Fragment, "Entities", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer EntitiesBuffer {\n" +
        "    Entity data[];\n" +
        "} Entities;\n";
    bindingIndex = shader.getSSBO_BindingIndex("Materials");
    shader.addSSBO(ShaderType::Fragment, "Materials", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer MaterialsBuffer {\n" +
        "    Material data[];\n" +
        "} Materials;\n";
    bindingIndex = shader.getSSBO_BindingIndex("MeshPositions");
    shader.addSSBO(ShaderType::Fragment, "MeshPositions", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer MeshPositionsBuffer {\n" +
        "    vec4 data[];\n" +
        "} MeshPositions;\n";
    // bindingIndex = shader.getSSBO_BindingIndex("MeshNormals");
    // shader.addSSBO(ShaderType::Fragment, "MeshNormals", bindingIndex);
    // string += "layout(std430, binding = " + std::to_string(bindingIndex) +
    //     ") buffer MeshNormalsBuffer {\n" +
    //     "    vec4 data[];\n" +
    //     "} MeshNormals;\n";
    bindingIndex = shader.getSSBO_BindingIndex("EntityUV2s");
    shader.addSSBO(ShaderType::Fragment, "EntityUV2s", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer EntityUV2sBuffer {\n" +
        "    vec2 data[];\n" +
        "} EntityUV2s;\n";
    bindingIndex = shader.getSSBO_BindingIndex("EntityUV3s");
    shader.addSSBO(ShaderType::Fragment, "EntityUV3s", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer EntityUV3sBuffer {\n" +
        "    vec3 data[];\n" +
        "} EntityUV3s;\n";
      return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "Color", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    return "layout(location = " + std::to_string(sf.currentInLayoutIndex++) +
      ") in vec4 inColor;";
  });
  sf.addHook(ShaderType::Fragment, "layout", "Color", [](auto& shader, const auto& constants)-> std::string
  {
    auto constantsEnd = constants.end();
    auto& sf = ShaderFactory::GetSingleton();
    if (std::find(constants.begin(), constantsEnd, "DepthMap") == constantsEnd)
    {
      return "layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
        ") out vec4 FragColor;";
    }
    else
    {
      return "vec4 FragColor;";
    }
  });
  sf.addHook(ShaderType::Fragment, "layout", "Position", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    return "layout(location = " + std::to_string(sf.currentInLayoutIndex++) +
      ") in vec4 inPosition;";
  });
  sf.addHook(ShaderType::Fragment, "layout", "UV2", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    return "layout(location = " + std::to_string(sf.currentInLayoutIndex++) +
      ") in vec2 inUV;";
  });
  sf.addHook(ShaderType::Fragment, "layout", "UV3", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    return "layout(location = " + std::to_string(sf.currentInLayoutIndex++) +
      ") in vec3 inUV;";
  });
  sf.addHook(ShaderType::Fragment, "layout", "CameraPosition", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addUBO(ShaderType::Fragment, "CameraPosition", bindingIndex, sizeof(glm::vec3));
    return "layout(binding = " + std::to_string(bindingIndex) + ") uniform CameraPositionBuffer {\n" +
      " vec3 value;\n" +
      "} CameraPosition;";
  });
  sf.addHook(ShaderType::Fragment, "layout", "View", [](auto& shader, const auto& constants)-> std::string
  {
    auto bindingIndex = shader.getSSBO_BindingIndex("InverseInstanceViews");
    shader.addSSBO(ShaderType::Fragment, "InverseInstanceViews", bindingIndex);
    std::string string;
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer InverseInstanceViewsBuffer {\n" +
        "    mat4 data[];\n" +
        "} InverseInstanceViews;\n";
    bindingIndex = shader.getSSBO_BindingIndex("InstanceViews");
    shader.addSSBO(ShaderType::Fragment, "InstanceViews", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer InstanceViewsBuffer {\n" +
        "    mat4 data[];\n" +
        "} InstanceViews;\n";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "Projection", [](auto& shader, const auto& constants)-> std::string
  {
    auto bindingIndex = shader.getSSBO_BindingIndex("InverseInstanceProjections");
    shader.addSSBO(ShaderType::Fragment, "InverseInstanceProjections", bindingIndex);
    std::string string;
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer InverseInstanceProjectionsBuffer {\n" +
        "    mat4 data[];\n" +
        "} InverseInstanceProjections;\n";
    bindingIndex = shader.getSSBO_BindingIndex("InstanceProjections");
    shader.addSSBO(ShaderType::Fragment, "InstanceProjections", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer InstanceProjectionsBuffer {\n" +
        "    mat4 data[];\n" +
        "} InstanceProjections;\n";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "Model", [](auto& shader, const auto& constants)-> std::string
  {
    auto bindingIndex = shader.getSSBO_BindingIndex("InverseInstanceModels");
    shader.addSSBO(ShaderType::Fragment, "InverseInstanceModels", bindingIndex);
    std::string string;
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer InverseInstanceModelsBuffer {\n" +
        "    mat4 data[];\n" +
        "} InverseInstanceModels;\n";
    bindingIndex = shader.getSSBO_BindingIndex("InstanceModels");
    shader.addSSBO(ShaderType::Fragment, "InstanceModels", bindingIndex);
    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer InstanceModelsBuffer {\n" +
        "    mat4 data[];\n" +
        "} InstanceModels;\n";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "Fog", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addUBO(ShaderType::Fragment, "FogDensity", bindingIndex, sizeof(glm::vec3));
    auto string = "layout(binding = " + std::to_string(bindingIndex) + ") uniform FogDensity {\n" +
      " float value;\n" +
      "} fogDensity;";
    bindingIndex = sf.currentBindingIndex++;
    shader.addUBO(ShaderType::Fragment, "FogColor", bindingIndex, sizeof(glm::vec3));
    string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform FogColor {\n" +
      " vec4 value;\n" +
      "} fogColor;";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "Normal", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto string = std::string(
      "layout(location = " + std::to_string(sf.currentInLayoutIndex++) +
      ") in vec4 inFragPosition;\n");
    string += "layout(location = " + std::to_string(sf.currentInLayoutIndex++) +
      ") in vec3 inNormal;\n" +
        "vec3 modNormal;";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "Lighting", [](auto& shader, const auto& constants)-> std::string
  {
    std::string string;
    string += std::string("struct PointLight{\n") +
      "  vec3 position;\n"
      "  vec3 color;\n"
      "  float intensity;\n"
      "  float range;\n"
      "  float nearPlane;\n"
      "  float farPlane;\n"
      "  float ambientFactor;\n"
      "};\n"
      "struct DirectionalLight{\n"
      "  vec3 position;\n"
      "  vec3 direction;\n"
      "  vec3 up;\n"
      "  vec3 color;\n"
      "  float intensity;\n"
      "  float nearPlane;\n"
      "  float farPlane;\n"
      "  float ambientFactor;\n"
      "};\n"
      "struct SpotLight{\n"
      "  vec3 position;\n"
      "  vec3 direction;\n"
      "  vec3 color;\n"
      "  float intensity;\n"
      "  float cutoff;\n"
      "  float outerCutoff;\n"
      "  float nearPlane;\n"
      "  float farPlane;\n"
      "  float ambientFactor;\n"
      "};\n";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "DirectionalLightShadowMaps", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Fragment, "DirectionalLights", bindingIndex);
    std::string string;
      string += "layout(std430, binding = " + std::to_string(bindingIndex) +
        ") buffer DirectionalLightBuffer {\n" +
        " DirectionalLight directionalLights[];\n" +
        "};\n";
    bindingIndex = sf.currentBindingIndex;
    shader.addTexture(bindingIndex, ShaderType::Fragment, "directionalLightSamplers", 1);
    string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler2D directionalLightSamplers[1];";
    sf.currentBindingIndex += 1;
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "SpotLightShadowMaps", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Fragment, "SpotLights", bindingIndex);
    std::string string;
    string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer SpotLightBuffer {\n"
      +
      " SpotLight spotLights[];\n" +
      "};";
    bindingIndex = sf.currentBindingIndex;
    shader.addTexture(bindingIndex, ShaderType::Fragment, "spotLightSamplers", 1);
    string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler2D spotLightSamplers[1];";
    sf.currentBindingIndex += 1;
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "PointLightShadowMaps", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addSSBO(ShaderType::Fragment, "PointLights", bindingIndex);
    std::string string;
    string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer PointLightBuffer {\n"
      +
      " PointLight pointLights[];\n" +
      "};\n";
    bindingIndex = sf.currentBindingIndex;
    shader.addTexture(bindingIndex, ShaderType::Fragment, "pointLightSamplers", 1);
    string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform samplerCube pointLightSamplers[1];";
    sf.currentBindingIndex += 1;
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "LightSpacePosition", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    std::string string(
      "layout(location = " + std::to_string(sf.currentInLayoutIndex) +
      ") in vec4 inDirectionalLightSpaceVertices[1];\n");
    sf.currentInLayoutIndex += 1;
    string += "layout(location = " + std::to_string(sf.currentInLayoutIndex) +
      ") in vec4 inSpotLightSpaceVertices[1];";
    sf.currentInLayoutIndex += 1;
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "PointLightSpaceMatrix", [](auto& shader, const auto& constants)-> std::string
  {
    return "uniform vec3 lightPos;\nuniform float nearPlane;\nuniform float farPlane;";
  });
  sf.addHook(ShaderType::Fragment, "layout", "ColorTexture", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    std::string string("layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
      ") out vec4 FragColor;\n");
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addTexture(bindingIndex, ShaderType::Fragment, "ColorTexture");
    string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler2D ColorTexture;";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "Texture3D", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    std::string string("layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
      ") out vec4 FragColor;\n");
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addTexture(bindingIndex, ShaderType::Fragment, "Texture3D");
    string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler3D Texture3D;";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "TextureCube", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    std::string string("layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
        ") out vec4 FragColor;\n");
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addTexture(bindingIndex, ShaderType::Fragment, "TextureCube");
    string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform samplerCube TextureCube;";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "layout", "TextColor", [](auto& shader, const auto& constants)-> std::string
  {
    auto& sf = ShaderFactory::GetSingleton();
    auto bindingIndex = sf.currentBindingIndex++;
    shader.addUBO(ShaderType::Fragment, "TextColor", bindingIndex, sizeof(glm::vec4));
    return "layout(binding = " + std::to_string(bindingIndex) + ") uniform TextColorBuffer {\n"
    "  vec4 value;\n"
    "} TextColor;";
  });
  sf.addHook(ShaderType::Fragment, "preMain", "Fog", [](auto& shader, const auto& constants)-> std::string
  {
    return "float calculateFogFactor(in float distance, in float density);";
  });
  sf.addHook(ShaderType::Fragment, "preMain", "Lighting", [](auto& shader, const auto& constants)-> std::string
  {
    return std::string(
        "vec3 calculatePointLight(in PointLight light, in vec3 fragPos, in vec3 normal, in vec3 viewDir, in float shadowFactor, in vec3 lightDir);\n")
      +
      "vec3 calculateDirectionalLight(in DirectionalLight light, in vec3 normal, in vec3 viewDir, in float shadowFactor, in vec3 lightDir);\n"
      +
      "vec3 calculateSpotLight(in SpotLight light, in vec3 fragPos, in vec3 normal, in vec3 viewDir, in float shadowFactor, in vec3 lightDir);";
  });
  sf.addHook(ShaderType::Fragment, "preMain", "DirectionalLightShadowMaps", [](auto& shader, const auto& constants)-> std::string
  {
    return
      "float calculateDirectionalLightShadowFactor(in vec4 lightSpacePosition, in sampler2D shadowMap, in vec3 normal, in vec3 lightDir, float near, float far);";
  });
  sf.addHook(ShaderType::Fragment, "preMain", "SpotLightShadowMaps", [](auto& shader, const auto& constants)-> std::string
  {
    return
      "float calculateSpotLightShadowFactor(in vec4 lightSpacePosition, in sampler2D shadowMap, in vec3 normal, in vec3 lightDir);";
  });
  sf.addHook(ShaderType::Fragment, "preMain", "PointLightShadowMaps", [](auto& shader, const auto& constants)-> std::string
  {
    return
      "float calculatePointLightShadowFactor(in vec3 fragPos, in samplerCube shadowMap, in vec3 lightPos, in float nearPlane, in float farPlane, in vec3 normal, in vec3 lightDir);";
  });
  sf.addHook(ShaderType::Fragment, "preMain", "MSDF", [](auto& shader, const auto& constants)-> std::string
  {
    return R"(float median(float r, float g, float b);
float screenPxRange();)";
  });
  sf.addHook(ShaderType::Fragment, "preInMain", "Shape", [](auto& shader, const auto& constants) -> std::string
  {
    return R"(  int entity_id = get_entity_id();
  Entity entity = get_entity(entity_id);
  Material material = get_material(entity);)";
  });
  sf.addHook(ShaderType::Fragment, "preInMain", "Color", [](auto& shader, const auto& constants)-> std::string
  {
    std::string string;
    string += R"(  vec4 entity_color = vec4(0, 0, 0, 0);
  if (entity.shape_type == SHAPE_TYPE_SDF) {
    entity_color = get_entity_color(entity, material);
  }
  else {
    entity_color = inColor;
  }
  if (entity_color.a <= 0.1)
    discard;
  float ndcZ = inPosition.z / inPosition.w;
  gl_FragDepth = 0.5 + 0.5 * ndcZ;
  FragColor = entity_color;
)";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "preInMain", "ColorTexture", [](auto& shader, const auto& constants)-> std::string
  {
    std::string string("  vec4 sampled = texture(ColorTexture, inUV);\n");
    auto constants_begin = constants.begin();
    auto constants_end = constants.end();
    if (std::find(constants_begin, constants_end, "TextColor") != constants_end)
    {
      string += R"(  if (sampled.a <= 0.1)
    discard;
  FragColor = vec4(TextColor.value.r, TextColor.value.g, TextColor.value.b, sampled.r * TextColor.value.a);)";
    }
    else if (std::find(constants_begin, constants_end, "MSDF") != constants_end)
    {
      string += R"(  float sd_msdf = median(sampled.r, sampled.g, sampled.b);
  float sd_tsdf = sampled.a;
  float blendRange = 0.5 / screenPxRange();
  float blendFactor = smoothstep(0.5 - blendRange, 0.5 + blendRange, sd_msdf);
  float sd = mix(sd_msdf, sd_tsdf, blendFactor);
  float screenPxDistance = screenPxRange()*(sd - 0.5);
  float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
  FragColor = mix(vec4(0, 0, 0, 0), vec4(1, 1, 1, 1), opacity);)";
    }
    else
    {
      string += "  FragColor = sampled;";
    }
    return string;
  });
  sf.addHook(ShaderType::Fragment, "preInMain", "Texture3D", [](auto& shader, const auto& constants)-> std::string
  {
    return "  FragColor = texture(Texture3D, inUV);";
  });
  sf.addHook(ShaderType::Fragment, "preInMain", "TextureCube", [](auto& shader, const auto& constants)-> std::string
  {
    std::string string = "  vec3 sampleDir = normalize(inUV);\n";
    string += "  sampleDir = clamp(sampleDir, -0.999, 0.999);\n";
    string += "  FragColor = texture(TextureCube, sampleDir);";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "preInMain", "PointLightSpaceMatrix", [](auto& shader, const auto& constants)-> std::string
  {
    return std::string("  float lightDistance = length(inFragPosition.xyz - lightPos);\n") +
      "  gl_FragDepth = lightDistance / farPlane;\n";
  });
  sf.addHook(ShaderType::Fragment, "postInMain", "Normal", [](auto& shader, const auto& constants)-> std::string
  {
    std::string string;
    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
      return constant == "Shape";
    });
    string += "  modNormal = get_entity_normal(entity, material);\n";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "postInMain", "Fog", [](auto& shader, const auto& constants)-> std::string
  {
    return std::string("  float distance = length(inPosition.xyz - CameraPosition.value);\n") +
      "  float fogFactor = calculateFogFactor(distance, fogDensity.value);\n"
      "  FragColor = mix(fogColor.value, FragColor, fogFactor);";
  });
  sf.addHook(ShaderType::Fragment, "postInMain", "Lighting", [](auto& shader, const auto& constants)-> std::string
  {
    return std::string("  vec3 normal = normalize(modNormal);\n") +
      "  vec3 viewDir = normalize(CameraPosition.value - inFragPosition.xyz);\n"
      "  vec3 lightingColor = vec3(0.0);\n";
  });
  sf.addHook(ShaderType::Fragment, "postInMain", "PointLightShadowMaps", [](auto& shader, const auto& constants)-> std::string
  {
    return  "  int pointLightCount = pointLights.length();\n"
            "  for (int i = 0; i < pointLightCount; ++i){\n"
            "    float shadowFactor = 0.0;"
            "    vec3 lightDir = normalize(pointLights[i].position - inFragPosition.xyz);\n"
            "    switch (i) {\n"
            "      case 0: shadowFactor = calculatePointLightShadowFactor(inFragPosition.xyz, pointLightSamplers[0], pointLights[i].position, pointLights[i].nearPlane, pointLights[i].farPlane, lightDir, normal); break;\n"
            // "      case 1: shadowFactor = calculatePointLightShadowFactor(inFragPosition.xyz, pointLightSamplers[1], pointLights[i].position, pointLights[i].nearPlane, pointLights[i].farPlane, lightDir, normal); break;\n"
            // "      case 2: shadowFactor = calculatePointLightShadowFactor(inFragPosition.xyz, pointLightSamplers[2], pointLights[i].position, pointLights[i].nearPlane, pointLights[i].farPlane, lightDir, normal); break;\n"
            // "      case 3: shadowFactor = calculatePointLightShadowFactor(inFragPosition.xyz, pointLightSamplers[3], pointLights[i].position, pointLights[i].nearPlane, pointLights[i].farPlane, lightDir, normal); break;\n"
            "    }\n"
            "    lightingColor += calculatePointLight(pointLights[i], inFragPosition.xyz, normal, viewDir, shadowFactor, lightDir);\n"
            "  }";
  });
  sf.addHook(ShaderType::Fragment, "postInMain", "DirectionalLightShadowMaps", [](auto& shader, const auto& constants)-> std::string
  {
    return  "  int directionalLightCount = directionalLights.length();\n"
            "  for (int i = 0; i < directionalLightCount; ++i){\n"
            "    vec3 lightDir = normalize(-directionalLights[i].direction);\n"
            "    float shadowFactor = 0.0;\n"
            "    switch (i) {\n"
            "      case 0: shadowFactor = calculateDirectionalLightShadowFactor(inDirectionalLightSpaceVertices[i], directionalLightSamplers[0], normal, lightDir, directionalLights[i].nearPlane, directionalLights[i].farPlane); break;\n"
            // "      case 1: shadowFactor = calculateDirectionalLightShadowFactor(inDirectionalLightSpaceVertices[i], directionalLightSamplers[1], normal, lightDir, directionalLights[i].nearPlane, directionalLights[i].farPlane); break;\n"
            // "      case 2: shadowFactor = calculateDirectionalLightShadowFactor(inDirectionalLightSpaceVertices[i], directionalLightSamplers[2], normal, lightDir, directionalLights[i].nearPlane, directionalLights[i].farPlane); break;\n"
            // "      case 3: shadowFactor = calculateDirectionalLightShadowFactor(inDirectionalLightSpaceVertices[i], directionalLightSamplers[3], normal, lightDir, directionalLights[i].nearPlane, directionalLights[i].farPlane); break;\n"
            "    }\n"
            "    lightingColor += calculateDirectionalLight(directionalLights[i], normal, viewDir, shadowFactor, lightDir);\n"
            "  }";
  });
  sf.addHook(ShaderType::Fragment, "postInMain", "SpotLightShadowMaps", [](auto& shader, const auto& constants)-> std::string
  {
    return  "  int spotLightCount = spotLights.length();\n"
            "  for (int i = 0; i < spotLightCount; ++i){\n"
            "    vec3 lightDir = normalize(spotLights[i].position - inFragPosition.xyz);\n"
            "    float shadowFactor = 0.0;"
            "    switch (i) {\n"
            "      case 0: shadowFactor = calculateSpotLightShadowFactor(inSpotLightSpaceVertices[i], spotLightSamplers[0], normal, lightDir); break;\n"
            // "      case 1: shadowFactor = calculateSpotLightShadowFactor(inSpotLightSpaceVertices[i], spotLightSamplers[1], normal, lightDir); break;\n"
            // "      case 2: shadowFactor = calculateSpotLightShadowFactor(inSpotLightSpaceVertices[i], spotLightSamplers[2], normal, lightDir); break;\n"
            // "      case 3: shadowFactor = calculateSpotLightShadowFactor(inSpotLightSpaceVertices[i], spotLightSamplers[3], normal, lightDir); break;\n"
            "    }\n"
            "    lightingColor += calculateSpotLight(spotLights[i], inFragPosition.xyz, normal, viewDir, shadowFactor, lightDir);\n"
            "  }";
  });
  sf.addHook(ShaderType::Fragment, "postPostInMain", "Lighting", [](auto& shader, const auto& constants)-> std::string
  {
    return "  FragColor = FragColor * vec4(lightingColor, 1.0);";
  });
  sf.addHook(ShaderType::Fragment, "postMain", "Shape", [](auto& shader, const auto& constants) -> std::string
  {
    std::string string;
    string += R"(
const float SDF_UNIT_EPSILON = 0.1;
const float PI = 3.14159265359;
const vec3 K_HEX_PRISM = vec3(-0.866025404, 0.5, 0.577350269); // -sqrt(3)/2, 1/2, 1/sqrt(3) for hexagon
const float SQRT3_DIV_4 = 0.43301270189; // sqrt(3)/4, inradius for unit hexagon with circumradius 0.5
Material get_material(in Entity entity)
{
  int material_index = entity.material_index;
  if (material_index >= 0) {
    return Materials.data[material_index];
  }
  Material defaultMaterial;
  defaultMaterial.type = 0;
  defaultMaterial.albedo = vec4(1.0);
  return defaultMaterial;
}
Entity get_entity(int entity_id)
{
  if (entity_id >= 0) {
    return Entities.data[entity_id];
  }
  Entity defaultEntity;
  defaultEntity.shape_type = -1;
  defaultEntity.material_index = -1;
  defaultEntity.vertex_offset = -1;
  defaultEntity.uv2_offset = -1;
  defaultEntity.uv3_offset = -1;
  return defaultEntity;
}
vec3 extractScale(in mat4 modelMatrix) {
  return vec3(modelMatrix[0][0], modelMatrix[1][1], modelMatrix[2][2]);
}
)";
  auto& sdf_rgy = SDFRegistry::GetSingleton();
  auto sdf_end = sdf_rgy.end();
  for (auto sdf_iter = sdf_rgy.begin(); sdf_iter != sdf_end; ++sdf_iter)
  {
    auto& key = sdf_iter->first;
    auto& tuple = sdf_iter->second;
    auto& id = std::get<0>(tuple);
    auto& function_string_callable = std::get<1>(tuple);
    string += function_string_callable(shader, constants) + "\n";
  }
  string += R"(float objectSDF(in Entity entity, vec3 p_world) {
  vec3 scale = extractScale(InstanceModels.data[inID]);
  vec3 p_object_local = vec3(InverseInstanceModels.data[inID] * vec4(p_world, 1.0));
  vec3 p_local_canonical = p_object_local;
  float distance;
  switch (entity.meta_int) {
)";
  for (auto sdf_iter = sdf_rgy.begin(); sdf_iter != sdf_end; ++sdf_iter)
  {
    auto& key = sdf_iter->first;
    auto& tuple = sdf_iter->second;
    auto& id = std::get<0>(tuple);
    auto& param_append_string = std::get<2>(tuple);
    string += "    case " + std::to_string(id) + R"(:
      distance = )" + key + "SDF(p_local_canonical" + (param_append_string.empty() ? "" : (", " + param_append_string)) + ");\n" +
      "      break;\n";
  }
  string += R"(    default:
        discard;
  }
  return distance;
  // vec3 scaled_p_local = p_local_canonical * scale;
  // float corrected_distance = distance * length(scaled_p_local) / length(p_local_canonical);
  // return corrected_distance;
}
const int MAX_STEPS = 150;
const float MIN_DIST = 0.005;
const float MAX_DIST = 10000.0;
vec3 unProjectToView(in vec3 win, in mat4 inverseProjection, in vec4 viewport)
{
    vec4 tmp = vec4(win, 1.0);
    tmp.x = (tmp.x - viewport.x) / viewport.z * 2.0 - 1.0;
    tmp.y = (tmp.y - viewport.y) / viewport.w * 2.0 - 1.0;
    tmp.z = win.z;
    vec4 viewPos = inverseProjection * tmp;
    viewPos /= viewPos.w;
    return vec3(viewPos);
}
vec4 sdf_get_color_shape(in Entity entity, vec4 baseColor)
{
)";
  auto constants_end = constants.end();
  if (std::find(constants.begin(), constants_end, "View") == constants_end)
  {
    string += "  return vec4(0);\n";
  }
  else
  {
    string += R"(
  vec2 screenCoord = gl_FragCoord.xy;
  mat4 inverseProjection = InverseInstanceProjections.data[inID];
  mat4 inverseView = InverseInstanceViews.data[inID];
  mat4 projection = InstanceProjections.data[inID];
  mat4 view = InstanceViews.data[inID];
  
  // // Transform the fragment position to view space
  // vec4 fragPosView = view * vec4(inFragPosition.xyz, 1.0);
  
  // // Nudge it closer to the camera by half a unit along the view Z-axis
  // fragPosView.z += 3.0; // Adjust this value as needed
  
  // // Transform the nudged view-space point back to world space
  // vec4 rayOriginWorld4 = inverseView * fragPosView;
  // rayOriginWorld4.xyz / rayOriginWorld4.w; // Perspective divide
  
  // Calculate the ray direction as before
  vec3 nearPointView = unProjectToView(vec3(screenCoord, 0.0), inverseProjection, Viewport.size);
  vec3 farPointView = unProjectToView(vec3(screenCoord, 0.99999), inverseProjection, Viewport.size);
  vec3 nearPointWorld = vec3(inverseView * vec4(nearPointView, 1.0));
  vec3 farPointWorld = vec3(inverseView * vec4(farPointView, 1.0));
  vec3 rayOrigin = inFragPosition.xyz;
  vec3 rayDirection = normalize(farPointWorld - nearPointWorld);
  float totalDistance = 0.0;
  vec3 currentPos = rayOrigin;
  float distanceToSurface = 0.0;
  bool hit = false; 
  for (int i = 0; i < MAX_STEPS; ++i) {
      distanceToSurface = objectSDF(entity, currentPos);
      if (abs(distanceToSurface) < MIN_DIST) {
          hit = true;
          break;
      }
      totalDistance += distanceToSurface;
      currentPos = rayOrigin + rayDirection * totalDistance;
      if (totalDistance > MAX_DIST) {
          break;
      }
  }
  if (hit) {
    vec3 normal;
    vec2 eps = vec2(0.001, 0.0);
    normal.x = objectSDF(entity, currentPos + eps.xyy) - objectSDF(entity, currentPos - eps.xyy);
    normal.y = objectSDF(entity, currentPos + eps.yxy) - objectSDF(entity, currentPos - eps.yxy);
    normal.z = objectSDF(entity, currentPos + eps.yyx) - objectSDF(entity, currentPos - eps.yyx);
    modNormal = normalize(normal);
    return baseColor;
  }
  discard;
)";
  }
  string += R"(
}
vec4 get_color(in Entity entity, in Material material)
{
)";
  if (std::find_if(constants.begin(), constants_end, [](auto& constant) {
    return constant == "UV2" || constant == "UV3";
  }) != constants_end)
  {
    string += "  vec4 inColor = texture(ColorTexture, inUV);\n";
  }
  string += R"(  switch (entity.shape_type) {
    case SHAPE_TYPE_SDF:
      return sdf_get_color_shape(entity, inColor);
    default:
      return inColor;
  }
  return vec4(0, 0, 0, 0);
}
vec3 get_normal(in Entity entity, in Material material)
{
)";
  if (std::find(constants.begin(), constants_end, "Normal") == constants_end)
  {
    string += "  return vec3(0, 0, 1);\n";
  }
  else
  {
    string += R"(
  switch (entity.shape_type) {
    case SHAPE_TYPE_SDF:
      return modNormal;
    default:
      return inNormal;
  }
)";
  }
  string += R"(
}
int get_entity_id()
{
  return inID;
};
vec4 get_entity_color(in Entity entity, in Material material)
{
  return get_color(entity, material);
}
vec3 get_entity_normal(in Entity entity, in Material material)
{
  return get_normal(entity, material);
};
)";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "postMain", "Fog", [](auto& shader, const auto& constants)-> std::string
  {
    return std::string("float calculateFogFactor(in float distance, in float density) {\n") +
      "  return exp(-density * distance);\n" +
      "}";
  });
  sf.addHook(ShaderType::Fragment, "postMain", "Lighting", [](auto& shader, const auto& constants)-> std::string
  {
    return std::string(
        "vec3 calculatePointLight(in PointLight light, in vec3 fragPos, in vec3 normal, in vec3 viewDir, in float shadowFactor, in vec3 lightDir){\n")
      +
      "  float distance = length(light.position - fragPos);\n" +
      "  float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));\n" +
      "  if (distance > light.range) attenuation = 0.0;\n" +
      "  float diff = max(dot(normal, lightDir), 0.0);\n" +
      "  vec3 reflectDir = reflect(-lightDir, normal);\n" +
      "  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);\n" +
      "  vec3 ambient = light.ambientFactor * light.color;\n" +
      "  vec3 diffuse = diff * light.color * light.intensity * attenuation * (1.0 - shadowFactor);\n" +
      "  vec3 specular = spec * light.color * light.intensity * attenuation * (1.0 - shadowFactor);\n" +
      "  return (ambient + diffuse + specular);\n" +
      "}\n" +
      "vec3 calculateDirectionalLight(in DirectionalLight light, in vec3 normal, in vec3 viewDir, in float shadowFactor, in vec3 lightDir){\n"
      +
      "  float diff = max(dot(normal, lightDir), 0.0);\n" +
      "  vec3 reflectDir = reflect(-lightDir, normal);\n" +
      "  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);\n" +
      "  vec3 ambient = light.ambientFactor * light.color;\n" +
      "  vec3 diffuse = diff * light.color * light.intensity * (1.0 - shadowFactor);\n" +
      "  vec3 specular = spec * light.color * light.intensity * (1.0 - shadowFactor);\n" +
      "  return ambient + diffuse + specular;\n" +
      "}\n" +
      "vec3 calculateSpotLight(in SpotLight light, in vec3 fragPos, in vec3 normal, in vec3 viewDir, in float shadowFactor, in vec3 lightDir){\n" +
      "  float theta = dot(lightDir, normalize(-light.direction));\n" +
      "  float epsilon = light.cutoff - light.outerCutoff;\n" +
      "  float intensity = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);\n" +
      "  if (theta < light.outerCutoff) intensity = 0.0;\n" +
      "  float diff = max(dot(normal, lightDir), 0.0);\n" +
      "  vec3 reflectDir = reflect(-lightDir, normal);\n" +
      "  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);\n" +
      "  vec3 ambient = light.ambientFactor * light.color;\n" +
      "  vec3 diffuse = diff * light.color * light.intensity * intensity * (1.0 - shadowFactor);\n" +
      "  vec3 specular = spec * light.color * light.intensity * intensity * (1.0 - shadowFactor);\n" +
      "  return ambient + diffuse + specular;\n" +
      "}";
  });
  sf.addHook(ShaderType::Fragment, "postMain", "DirectionalLightShadowMaps", [](auto& shader, const auto& constants)-> std::string
  {
    // Start building the GLSL function string
    std::string string = R"(
      float calculateDirectionalLightShadowFactor(
          in vec4 lightSpacePosition,
          in sampler2D shadowMap,
          in vec3 normal,
          in vec3 lightDir,
          in float nearPlane,
          in float farPlane
      ) {
          vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;
      )";
      if (shader.iRenderer->renderer == RENDERER_VULKAN) {
          string += "    projCoords.xy = projCoords.xy * 0.5 + 0.5;\n";
      } else {
          string += "    projCoords = projCoords * 0.5 + 0.5;\n";
      }
      string += R"(
          float currentDepth = projCoords.z;
          float shadowFactor = 0.0;
          vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
          float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);
          int kernelSize = 1;
          int radius = (kernelSize - 1) / 2;
          for(int x = -radius; x <= radius; ++x) {
              for(int y = -radius; y <= radius; ++y) {
                  vec2 offsetCoords = projCoords.xy + vec2(x, y) * texelSize;
                  float closestDepth = texture(shadowMap, offsetCoords).r;
                  shadowFactor += (currentDepth - bias) > closestDepth ? 1.0 : 0.0;
              }
          }
          shadowFactor /= float(kernelSize * kernelSize);
          return clamp(shadowFactor, 0.0, 1.0);
      }
      )";
      return string;
  });
  sf.addHook(ShaderType::Fragment, "postMain", "SpotLightShadowMaps", [](auto& shader, const auto& constants)-> std::string
  {
    std::string string(
        "float calculateSpotLightShadowFactor(in vec4 lightSpacePosition, in sampler2D shadowMap, in vec3 normal, in vec3 lightDir){\n");
    string += "  vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;\n";
    if (shader.iRenderer->renderer != RENDERER_VULKAN)
    {
      string += "  projCoords = projCoords * 0.5 + 0.5;\n";
    }
    string += "  float closestDepth = texture(shadowMap, projCoords.xy).r;\n"
    "  float currentDepth = projCoords.z;\n"
    "  float bias = max(0.00001 * (1.0 - dot(normal, lightDir)), 0.00001);\n"
    "  float shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;\n"
    "  return shadow;\n"
    "}";
    return string;
  });
  sf.addHook(ShaderType::Fragment, "postMain", "PointLightShadowMaps", [](auto& shader, const auto& constants)-> std::string
  {
    return std::string("float calculatePointLightShadowFactor(in vec3 fragPos, in samplerCube shadowMap, in vec3 lightPos, in float nearPlane, in float farPlane, in vec3 normal, in vec3 lightDir){\n") +
      "  vec3 lightToFrag = fragPos - lightPos;\n" +
      "  float currentDepth = length(lightToFrag) / farPlane;\n" +
      "  float closestDepth = texture(shadowMap, normalize(lightToFrag)).r;\n" +
      "  float bias = 0.005;\n" +
      "  return (currentDepth - bias) > closestDepth ? 1.0 : 0.0;\n" +
      "}";
  });
  sf.addHook(ShaderType::Fragment, "postMain", "MSDF", [](auto& shader, const auto& constants)-> std::string
  {
    return R"(float median(float r, float g, float b) {
  return max(min(r, g), min(max(r, g), b));
}
float pxRange = 2.0;
float screenPxRange() {
    vec2 unitRange = vec2(pxRange)/vec2(textureSize(ColorTexture, 0));
    vec2 screenTexSize = vec2(1.0)/fwidth(inUV);
    return max(0.5*dot(unitRange, screenTexSize), 1.0);
})";
  });
  registered_zg_shader_hooks = true;
}
ShaderFactory::ShaderTypeMap ShaderFactory::shaderTypes = {
#if defined(USE_GL) || defined(USE_EGL)
	{ShaderType::Vertex, GL_VERTEX_SHADER},
	{ShaderType::Geometry, GL_GEOMETRY_SHADER},
	{ShaderType::Fragment, GL_FRAGMENT_SHADER}
#ifdef USE_GL
	,
	{ShaderType::TessellationControl, GL_TESS_CONTROL_SHADER},
	{ShaderType::TessellationEvaluation, GL_TESS_EVALUATION_SHADER},
	{ShaderType::Compute, GL_COMPUTE_SHADER}
#endif
#endif
};
ShaderFactory::ShaderNameMap ShaderFactory::shaderNames = {
	{ShaderType::Vertex, "VertexShader"},
	{ShaderType::Geometry, "GeometryShader"},
	{ShaderType::Fragment, "FragmentShader"},
	{ShaderType::TessellationControl, "TessellationControlShader"},
	{ShaderType::TessellationEvaluation, "TessellationEvaluationShader"},
	{ShaderType::Compute, "ComputeShader"}};
ShaderMap ShaderFactory::generateShaderMap(const RuntimeConstants& constants, Shader& shader,
																					 const std::vector<ShaderType>& shaderTypes)
{
	ShaderMap shaderMap;
  currentBindingIndex = 0;
	for (auto& shaderType : shaderTypes)
	{
		shaderMap[shaderType] = generateShader(shaderType, constants, shader);
	}
	return shaderMap;
}
ShaderPair ShaderFactory::generateShader(const ShaderType& shaderType, const RuntimeConstants& constants,
																				 Shader& shader)
{
	ShaderPair shaderPair;
	auto& shaderString = shaderPair.first;
	auto& shaderHooks = hooks[shaderType];
	// #if defined(USE_GL) || defined(USE_VULKAN)
	shaderString += "#version 460 core\n"
                  "#extension GL_ARB_shader_draw_parameters : require\n"
	                "precision highp float;\n"
	                "precision highp samplerCube;\n";
	// #elif defined(USE_EGL)
	//   shaderString += "#version 310 es\n";
	//   if (shaderType == ShaderType::Geometry)
	//   {
	//     const char* extensions = (const char*)glGetString(GL_EXTENSIONS);
	//     if (strstr(extensions, "GL_EXT_geometry_shader") == NULL)
	//     {
	//       printf("GL_EXT_geometry_shader not supported!\n");
	//     }
	//     else
	//     {
	//       shaderString += "#extension GL_EXT_geometry_shader : require\n";
	//       printf("GL_EXT_geometry_shader is supported.\n");
	//     }
	//     if (strstr(extensions, "GL_OES_geometry_shader") == NULL)
	//     {
	//       printf("GL_OES_geometry_shader not supported!\n");
	//     }
	//     else
	//     {
	//       shaderString += "#extension GL_OES_geometry_shader : require\n";
	//       printf("GL_OES_geometry_shader is supported.\n");
	//     }
	//   }
	//   shaderString += "precision mediump float;\n";
	//   shaderString += "precision mediump samplerCube;\n";
	// #endif
	currentInLayoutIndex = 0;
	currentOutLayoutIndex = 0;
	appendHooks(shaderString, shaderHooks["preLayout"], constants, shader);
	appendHooks(shaderString, shaderHooks["layout"], constants, shader);
	appendHooks(shaderString, shaderHooks["preMain"], constants, shader);
	shaderString += "void main()\n{\n";
	appendHooks(shaderString, shaderHooks["preInMain"], constants, shader);
	appendHooks(shaderString, shaderHooks["postInMain"], constants, shader);
	appendHooks(shaderString, shaderHooks["postPostInMain"], constants, shader);
	shaderString += "}\n";
	appendHooks(shaderString, shaderHooks["postMain"], constants, shader);
	if (!compileShader(shader, shaderType, shaderPair))
	{
		zg::Logger::print(zg::Logger::Blank, "Errored Shader\n\n", shaderString);
		// #ifndef USE_VULKAN
		throw std::runtime_error("Failed to compile fragment shader");
		// #endif
	}
	return shaderPair;
}
void ShaderFactory::appendHooks(std::string& shaderString, RuntimeHooksMap& runtimeHooks,
																const RuntimeConstants& constants, Shader& shader)
{
	for (const auto& constant : constants)
	{
		const auto& constantHooks = runtimeHooks[constant];
		for (auto& hook : constantHooks)
		{
			shaderString += hook.second(shader, constants) + "\n";
		}
	}
}
bool ShaderFactory::compileShader(Shader& shader, ShaderType shaderType, ShaderPair& shaderPair)
{
  {
    static auto shadersPath = zgfilesystem::File::getProgramDirectoryPath() / "shaders";
    zgfilesystem::Directory::ensureExists(shadersPath);
    zgfilesystem::File shaderFile(shadersPath / ("shader-" + std::to_string(shader.hash) + "-" + shaderTypeStringMap[shaderType] + ".glsl"), enums::EFileLocation::Absolute, "w");
    shaderFile.writeBytes(0, shaderPair.first.size(), shaderPair.first.c_str());
  }
	return shader.iRenderer->compileShader(shader, shaderType, shaderPair);
}
bool ShaderFactory::compileProgram(Shader& shader) { return shader.iRenderer->compileProgram(shader); }
uint32_t ShaderFactory::addHook(const ShaderType& shaderType, const std::string& hookName,
																const std::string& runtimeConstant, const Shader::ShaderHook& hook)
{
	auto id = ++hooksCount;
	hooks[shaderType][hookName][runtimeConstant].emplace(id, hook);
	shaderHookInfos[id] = {shaderType, hookName, runtimeConstant};
	return id;
}
void ShaderFactory::deleteHook(uint32_t id)
{
	auto infoIter = shaderHookInfos.find(id);
	if (infoIter == shaderHookInfos.end())
		return;
	auto hooksIter = hooks.find(std::get<0>(infoIter->second));
	if (hooksIter == hooks.end())
		return;
	auto hookIter = hooksIter->second.find(std::get<1>(infoIter->second));
	if (hookIter == hooksIter->second.end())
		return;
	auto runtimeHookIter = hookIter->second.find(std::get<2>(infoIter->second));
	if (runtimeHookIter == hookIter->second.end())
		return;
	runtimeHookIter->second.erase(id);
}
