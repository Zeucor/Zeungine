#include <glm/fwd.hpp>
#include <stdexcept>
#include <zg/Logger.hpp>
#include <zg/Window.hpp>
#include <zg/lights/Lights.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/zgfilesystem/File.hpp>
#include <zg/zgfilesystem/Directory.hpp>
using namespace zg::shaders;
ShaderFactory::ShaderHooksMap ShaderFactory::hooks = {
  {
    ShaderType::Vertex, {
      {
        "layout", {
          {
            "Color", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
                    return constant == "Shape";
                  });
                  if (found == constants.end())
                  {
                    return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                      ") in vec4 inColor;";
                  }
                  return "";
                }
              },
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end() ||
                      std::find(constants.begin(), constants.end(), "DepthMap") != constants.end())
                  {
                    return "";
                  }
                  else
                  {
                    return "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                      ") out vec4 outColor;";
                  }
                }
              }
            }
          },
          {
            "Position", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
                    return constant == "Shape";
                  });
                  if (found == constants.end())
                  {
                    return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                      ") in vec3 inPosition;";
                  }
                  return "";
                }
              },
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end())
                  {
                    return "";
                  }
                  else
                  {
                    return "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                      ") out vec4 outPosition;";
                  }
                }
              }
            }
          },
          {
            "UV2", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
                    return constant == "Shape";
                  });
                  if (found == constants.end())
                  {
                    return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                      ") in vec2 inUV;";
                  }
                  return "";
                }
              },
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  return "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                    ") out vec2 outUV;";
                }
              }
            }
          },
          {
            "UV3", {
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
                      return constant == "Shape";
                    });
                    if (found == constants.end())
                    {
                      return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                        ") in vec3 inUV;";
                    }
                    return "";
                  }
                },
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    return "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                      ") out vec3 outUV;";
                  }
                }
            }
          },
          {
            "Shape", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants) -> std::string
                {
                  std::string string;
                  string += "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                    ") out int outID;";
                  string += R"(struct Entity {
  int shape_type;
  int material_index;
  int vertex_offset;
  int normal_offset;
  int uv2_offset;
  int uv3_offset;
};
const int SHAPE_TYPE_BOX = 1;
const int SHAPE_TYPE_PLANE = 2;
const int SHAPE_TYPE_SPHERE = 3;
const int SHAPE_TYPE_MESH = 4;
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
vec3 get_plane_vertex_xz(int vertex_id);
vec3 get_plane_normal_xz(int vertex_id);
vec3 get_plane_vertex_xy(int vertex_id);
vec3 get_plane_normal_xy(int vertex_id);
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
                  auto bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Vertex, "Entities", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) +
                      ") buffer EntitiesBuffer {\n" +
                      "    Entity data[];\n" +
                      "} Entities;\n";
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Vertex, "Materials", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) +
                      ") buffer MaterialsBuffer {\n" +
                      "    Material data[];\n" +
                      "} Materials;\n";
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Vertex, "MeshPositions", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) +
                      ") buffer MeshPositionsBuffer {\n" +
                      "    vec3 data[];\n" +
                      "} MeshPositions;\n";
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Vertex, "MeshNormals", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) +
                      ") buffer MeshNormalsBuffer {\n" +
                      "    vec3 data[];\n" +
                      "} MeshNormals;\n";
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Vertex, "EntityUV2s", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) +
                      ") buffer EntityUV2sBuffer {\n" +
                      "    vec2 data[];\n" +
                      "} EntityUV2s;\n";
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Vertex, "EntityUV3s", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) +
                      ") buffer EntityUV3sBuffer {\n" +
                      "    vec3 data[];\n" +
                      "} EntityUV3s;\n";
                    return string;
                }
              }
            }
          },
          {
            "View",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Vertex, "InverseInstanceViews", bindingIndex);
                  std::string string;
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer InverseInstanceViewsBuffer {\n" +
                      "    mat4 data[];\n" +
                      "} InverseInstanceViews;\n";
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Vertex, "InstanceViews", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer InstanceViewsBuffer {\n" +
                      "    mat4 data[];\n" +
                      "} InstanceViews;\n";
                  return string;
                }
              }
            }
          },
          {
            "Projection",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Vertex, "InverseInstanceProjections", bindingIndex);
                  std::string string;
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer InverseInstanceProjectionsBuffer {\n" +
                      "    mat4 data[];\n" +
                      "} InverseInstanceProjections;\n";
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Vertex, "InstanceProjections", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer InstanceProjectionsBuffer {\n" +
                      "    mat4 data[];\n" +
                      "} InstanceProjections;\n";
                  return string;
                }
              }
            }
          },
          {
            "Model",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Vertex, "InverseInstanceModels", bindingIndex);
                  std::string string;
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer InverseInstanceModelsBuffer {\n" +
                      "    mat4 data[];\n" +
                      "} InverseInstanceModels;\n";
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Vertex, "InstanceModels", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer InstanceModelsBuffer {\n" +
                      "    mat4 data[];\n" +
                      "} InstanceModels;\n";
                  return string;
                }
              }
            }
          },
          {
            "Normal", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
                    return constant == "Shape";
                  });
                  if (found == constants.end())
                  {
                    return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                      ") in vec3 inNormal;";
                  }
                  return "";
                }
              },
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto string = std::string(
                    "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                    ") out vec4 outFragPosition;\n");
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end())
                  {
                    return string;
                  }
                  string += "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                    ") out vec3 outNormal;";
                  return string;
                }
              }
            }
          },
          {
            "LightSpaceMatrix",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addUBO(ShaderType::Vertex, "LightSpaceMatrix", bindingIndex, sizeof(glm::mat4));
                  return "layout(binding = " + std::to_string(bindingIndex) + ") uniform LightSpaceMatrix {\n" +
                    "  mat4 matrix;\n" +
                    "} lightSpaceMatrix;";
                }
              }
            }
          },
          {
            "LightSpacePosition", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  std::string string;
                  auto bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addUBO(ShaderType::Vertex, "DirectionalLightSpaceMatrices", bindingIndex, sizeof(glm::mat4));
                  string += "layout(binding = " + std::to_string(bindingIndex) +
                    ") uniform DirectionalLightSpaceMatrices {\n" +
                    " mat4 matrix[1];\n" +
                    "} directionalLightSpaceMatrices;\n";
                  string += "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex) +
                    ") out vec4 outDirectionalLightSpaceVertices[1];\n";
                  ShaderFactory::currentOutLayoutIndex += 1;
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addUBO(ShaderType::Vertex, "SpotLightSpaceMatrices", bindingIndex, sizeof(glm::mat4));
                  string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform SpotLightSpaceMatrices {\n"
                    +
                    "  mat4 matrix[1];\n" +
                    "} spotLightSpaceMatrices;\n";
                  string += "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex) +
                    ") out vec4 outSpotLightSpaceVertices[1];";
                  ShaderFactory::currentOutLayoutIndex += 1;
                  return string;
                }
              }
            }
          }
        }
      },
      {
        "preInMain", {
          {
            "Shape", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants) -> std::string
                {
                  return R"(int entity_id = get_entity_id();
  outID = entity_id;
  Entity entity = get_entity(entity_id);
  Material material = get_material(entity);)";
                }
              }
            }
          },
          {
            "Color", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end() ||
                      std::find(constants.begin(), constants.end(), "DepthMap") != constants.end())
                  {
                    return "";
                  }
                  else
                  {
                    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
                      return constant == "Shape";
                    });
                    if (found == constants.end())
                    {
                      return "  outColor = inColor;";
                    }
                    return "  outColor = get_entity_color(gl_VertexIndex, entity, material);";
                  }
                }
              }
            }
          },
          {
            "Position", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const RuntimeConstants& constants)-> std::string
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
                  if (std::find(constants.begin(), constants.end(), "LightSpaceMatrix") != constants.end())
                  {
                    string += "lightSpaceMatrix.matrix * ";
                  }
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
                  if (!(std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end()))
                  {
                    string += "  outPosition = gl_Position;";
                  }
                  return string;
                }
              }
            }
          },
          {
            "LightSpacePosition", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const RuntimeConstants& constants)-> std::string
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
                }
              }
            }
          },
          {
            "Normal", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  std::string string;
                  auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
                    return constant == "Shape";
                  });
                  if (found != constants.end())
                  {
                    string += "  vec3 inNormal = get_entity_normal(gl_VertexIndex, entity, material);\n";
                  }
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end())
                  {
                    return string;
                  }
                  string += "  outFragPosition = InstanceModels.data[gl_InstanceIndex] * vec4(inPosition, 1.0);\n";
                  string += "  outNormal = mat3(transpose(inverse(InstanceModels.data[gl_InstanceIndex]))) * inNormal;";
                  return string;
                }
              }
            }
          },
          {
            "UV2", {
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
                      return constant == "Shape";
                    });
                    if (found != constants.end())
                    {
                      return "  outUV = get_entity_uv2(gl_VertexIndex, entity, material);";
                    }
                    return "  outUV = inUV;";
                  }
                }
            }
          },
          {
            "UV3", {
                  {
                    ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                    {
                      auto found = std::find_if(constants.begin(), constants.end(), [](auto& constant) {
                        return constant == "Shape";
                      });
                      if (found != constants.end())
                      {
                        return "  outUV = normalize(get_entity_uv3(gl_VertexIndex, entity, material));";
                      }
                      return "  outUV = inUV;";
                    }
                  }
            }
          }
        }
      },
      {
        "postMain",
        {
          {
            "Shape", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants) -> std::string
                {
                  return R"(
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
vec2 get_uv2(int vertex_id, in Entity entity, in Material material)
{
  if (material.type != 1)
    return vec2(0, 0);
  return EntityUV2s.data[entity.uv2_offset + vertex_id];
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
  vec3 v0 = vec3(-1.0, -1.0, -1.0); // NLL
  vec3 v1 = vec3( 1.0, -1.0, -1.0); // NLR
  vec3 v2 = vec3(-1.0,  1.0, -1.0); // NUL
  vec3 v3 = vec3( 1.0,  1.0, -1.0); // NUR
  vec3 v4 = vec3(-1.0, -1.0,  1.0); // FLL
  vec3 v5 = vec3( 1.0, -1.0,  1.0); // FLR
  vec3 v6 = vec3(-1.0,  1.0,  1.0); // FUL
  vec3 v7 = vec3( 1.0,  1.0,  1.0); // FUR

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
vec3 get_plane_vertex_xz(int vertex_id)
{
  switch (vertex_id)
  {
    case 0: return vec3(-1.0, 0.0, -1.0);
    case 1: return vec3( 1.0, 0.0, -1.0);
    case 2: return vec3(-1.0, 0.0,  1.0);
    case 3: return vec3(-1.0, 0.0,  1.0);
    case 4: return vec3( 1.0, 0.0, -1.0);
    case 5: return vec3( 1.0, 0.0,  1.0);
    default: return vec3(0.0, 0.0, 0.0);
  }
}
vec3 get_plane_vertex_xy(int vertex_id)
{
  switch (vertex_id)
  {
    case 0: return vec3(-0.5, -0.5, 0.0);
    case 1: return vec3( 0.5, -0.5, 0.0);
    case 2: return vec3(-0.5,  0.5, 0.0);
    case 3: return vec3(-0.5,  0.5, 0.0);
    case 4: return vec3( 0.5, -0.5, 0.0);
    case 5: return vec3( 0.5,  0.5, 0.0);
    default: return vec3(0.0, 0.0, 0.0);
  }
}
vec3 get_plane_normal_xy(int vertex_id)
{
  return vec3(0.0, 0.0, 1.0);
}
vec3 get_plane_normal_xz(int vertex_id)
{
    return vec3(0, 1, 0);
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
  return MeshPositions.data[entity.vertex_offset + vertex_id];
}
vec3 get_mesh_normal(int vertex_id, in Entity entity, in Material material)
{
  return MeshNormals.data[entity.vertex_offset + vertex_id];
}
vec2 get_mesh_uv2(int vertex_id, in Entity entity, in Material material)
{
  if (material.type == 2)
  {
    return get_uv2(vertex_id, entity, material);
  }
  return vec2(0, 0);
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
  case SHAPE_TYPE_PLANE:
    return get_plane_vertex_xz(vertex_id);
  case SHAPE_TYPE_SPHERE:
  {
    vec3 vertex_local = get_plane_vertex_xy(vertex_id) * 2.0;
    mat4 viewMatrix = InstanceViews.data[gl_InstanceIndex];
    mat3 viewRotationInverse = transpose(mat3(viewMatrix));
    return viewRotationInverse * vertex_local;
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
  case SHAPE_TYPE_PLANE:
    return get_plane_normal_xz(vertex_id);
  case SHAPE_TYPE_SPHERE:
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
                }
              }
            }
          }
        }
      }
    }
  },
  {
    ShaderType::Geometry, {
      {
        "preLayout",
        {
          {
            "Position", {
              {
                ++ShaderFactory::hooksCount, [](auto &shader, const auto &constants)->std::string
                {
                  return std::string("layout(triangles) in;\n") +
                    "layout(triangle_strip, max_vertices = 18) out;";
                }
              }
            }
          }
        }
      },
      {
        "layout", {
            {
              "PointLightSpaceMatrix",
              {
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    auto bindingIndex = ShaderFactory::currentBindingIndex++;
                    shader.addUBO(ShaderType::Geometry, "PointLightSpaceMatrix", bindingIndex, sizeof(glm::mat4));
                    return "layout(binding = " + std::to_string(bindingIndex) + ") uniform PointLightSpaceMatrix {\n" +
                      "  mat4 matrix[6];\n" +
                      "} pointLightSpaceMatrix;";
                  }
                }
              }
            },
            {
              "Position", {
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    return "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                      ") out vec4 outFragPosition;";
                  }
                }
              }
            }
        }
      },
      {
        "preInMain",
        {
            {
              "PointLightSpaceMatrix", {
                {
                  ++ShaderFactory::hooksCount, [](auto &shader, const auto &constants)->std::string
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
                  }
                }
              }
            }
        }
      }
    }
  },
  {
    ShaderType::Fragment, {
      {
        "layout",
        {
          {
            "Shape", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants) -> std::string
                {
                  std::string string;
                  string += "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                    ") flat in int inID;\n";
                  string += R"(struct Entity {
  int shape_type;
  int material_index;
  int vertex_offset;
  int normal_offset;
  int uv2_offset;
  int uv3_offset;
};
const int SHAPE_TYPE_BOX = 1;
const int SHAPE_TYPE_PLANE = 2;
const int SHAPE_TYPE_SPHERE = 3;
const int SHAPE_TYPE_MESH = 4;
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
                      "    vec3 data[];\n" +
                      "} MeshPositions;\n";
                  bindingIndex = shader.getSSBO_BindingIndex("MeshNormals");
                  shader.addSSBO(ShaderType::Fragment, "MeshNormals", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) +
                      ") buffer MeshNormalsBuffer {\n" +
                      "    vec3 data[];\n" +
                      "} MeshNormals;\n";
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
                }
              }
            }
          },
          {
            "Color", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end() ||
                      std::find(constants.begin(), constants.end(), "DepthMap") != constants.end())
                  {
                    return "";
                  }
                  else
                  {
                    return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                      ") in vec4 inColor;";
                  }
                },
              },
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end() ||
                      std::find(constants.begin(), constants.end(), "DepthMap") != constants.end())
                  {
                    return "";
                  }
                  else
                  {
                    return "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                      ") out vec4 FragColor;";
                  }
                }
              }
            }
          },
          {
            "Position", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end())
                  {
                    return "";
                  }
                  else
                  {
                    return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                      ") in vec4 inPosition;";
                  }
                }
              }
            }
          },
          {
            "UV2", {
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                      ") in vec2 inUV;";
                  }
                }
            }
          },
          {
            "UV3", {
                  {
                    ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                    {
                      return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                        ") in vec3 inUV;";
                    }
                  }
            }
          },
          {
            "CameraPosition",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addUBO(ShaderType::Fragment, "CameraPosition", bindingIndex, sizeof(glm::vec3));
                  return "layout(binding = " + std::to_string(bindingIndex) + ") uniform CameraPositionBuffer {\n" +
                    " vec3 value;\n" +
                    "} CameraPosition;";
                }
              }
            }
          },
          {
            "View",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
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
                }
              }
            }
          },
          {
            "Projection",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
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
                }
              }
            }
          },
          {
            "Model",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
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
                }
              }
            }
          },
          {
            "Fog", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addUBO(ShaderType::Fragment, "FogDensity", bindingIndex, sizeof(glm::vec3));
                  auto string = "layout(binding = " + std::to_string(bindingIndex) + ") uniform FogDensity {\n" +
                    " float value;\n" +
                    "} fogDensity;";
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addUBO(ShaderType::Fragment, "FogColor", bindingIndex, sizeof(glm::vec3));
                  string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform FogColor {\n" +
                    " vec4 value;\n" +
                    "} fogColor;";
                  return string;
                }
              }
            }
          },
          {
            "Normal", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto string = std::string(
                    "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                    ") in vec4 inFragPosition;\n");
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end())
                  {
                    return string;
                  }
                  string += "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                    ") in vec3 inNormal;";
                  return string;
                }
              }
            }
          },
          {
            "Lighting",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
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
                }
              }
            }
          },
          {
            "DirectionalLightShadowMaps",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Fragment, "DirectionalLights", bindingIndex);
                  std::string string;
                    string += "layout(std430, binding = " + std::to_string(bindingIndex) +
                      ") buffer DirectionalLightBuffer {\n" +
                      " DirectionalLight directionalLights[];\n" +
                      "};\n";
                  bindingIndex = ShaderFactory::currentBindingIndex;
                  shader.addTexture(bindingIndex, ShaderType::Fragment, "directionalLightSamplers", 1);
                  string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler2D directionalLightSamplers[1];";
                  currentBindingIndex += 1;
                  return string;
                }
              }
            }
          },
          {
            "SpotLightShadowMaps",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO(ShaderType::Fragment, "SpotLights", bindingIndex);
                  std::string string;
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer SpotLightBuffer {\n"
                    +
                    " SpotLight spotLights[];\n" +
                    "};";
                  bindingIndex = ShaderFactory::currentBindingIndex;
                  shader.addTexture(bindingIndex, ShaderType::Fragment, "spotLightSamplers", 1);
                  string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler2D spotLightSamplers[1];";
                  currentBindingIndex += 1;
                  return string;
                }
              }
            }
          },
          {
            "PointLightShadowMaps",
            {
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    auto bindingIndex = ShaderFactory::currentBindingIndex++;
                    shader.addSSBO(ShaderType::Fragment, "PointLights", bindingIndex);
                    std::string string;
                    string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer PointLightBuffer {\n"
                      +
                      " PointLight pointLights[];\n" +
                      "};\n";
                    bindingIndex = ShaderFactory::currentBindingIndex;
                    shader.addTexture(bindingIndex, ShaderType::Fragment, "pointLightSamplers", 1);
                    string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform samplerCube pointLightSamplers[1];";
                    currentBindingIndex += 1;
                    return string;
                  }
                }
            }
          },
          {
            "LightSpacePosition", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  std::string string(
                    "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex) +
                    ") in vec4 inDirectionalLightSpaceVertices[1];\n");
                  ShaderFactory::currentInLayoutIndex += 1;
                  string += "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex) +
                    ") in vec4 inSpotLightSpaceVertices[1];";
                  ShaderFactory::currentInLayoutIndex += 1;
                  return string;
                }
              }
            }
          },
          {
            "PointLightSpaceMatrix", {
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    return std::string("uniform vec3 lightPos;\nuniform float nearPlane;\nuniform float farPlane;");
                  }
                }
            }
          },
          {
            "ColorTexture", {
                    {
                      ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                      {
                        std::string string("layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                          ") out vec4 FragColor;\n");
                        auto bindingIndex = ShaderFactory::currentBindingIndex++;
                        shader.addTexture(bindingIndex, ShaderType::Fragment, "ColorTexture");
                        string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler2D ColorTexture;";
                        return string;
                      }
                    }
            }
          },
          {
            "Texture3D", {
                    {
                      ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                      {
                        std::string string("layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                          ") out vec4 FragColor;\n");
                        auto bindingIndex = ShaderFactory::currentBindingIndex++;
                        shader.addTexture(bindingIndex, ShaderType::Fragment, "Texture3D");
                        string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler3D Texture3D;";
                        return string;
                      }
                    }
            }
          },
          {
            "TextureCube", {
                  {
                    ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                    {
                      std::string string("layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                          ") out vec4 FragColor;\n");
                      auto bindingIndex = ShaderFactory::currentBindingIndex++;
                      shader.addTexture(bindingIndex, ShaderType::Fragment, "TextureCube");
                      string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform samplerCube TextureCube;";
                      return string;
                    }
                  }
            }
          },
					{
						"TextColor", {
	                {
	                	++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
	                	{
                      auto bindingIndex = ShaderFactory::currentBindingIndex++;
                      shader.addUBO(ShaderType::Fragment, "TextColor", bindingIndex, sizeof(glm::vec4));
	                		return "layout(binding = " + std::to_string(bindingIndex) + ") uniform TextColor {\n"
                      "  vec4 value;\n"
                      "} textColor;";
	                	}
	                }
						}
					}
        }
      },
      {
        "preMain", {
          {
            "Fog", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  return "float calculateFogFactor(in float distance, in float density);";
                }
              }
            }
          },
          {
            "Lighting", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  return std::string(
                      "vec3 calculatePointLight(in PointLight light, in vec3 fragPos, in vec3 normal, in vec3 viewDir, in float shadowFactor, in vec3 lightDir);\n")
                    +
                    "vec3 calculateDirectionalLight(in DirectionalLight light, in vec3 normal, in vec3 viewDir, in float shadowFactor, in vec3 lightDir);\n"
                    +
                    "vec3 calculateSpotLight(in SpotLight light, in vec3 fragPos, in vec3 normal, in vec3 viewDir, in float shadowFactor, in vec3 lightDir);";
                }
              }
            }
          },
          {
            "DirectionalLightShadowMaps",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  return
                    "float calculateDirectionalLightShadowFactor(in vec4 lightSpacePosition, in sampler2D shadowMap, in vec3 normal, in vec3 lightDir, float near, float far);";
                }
              }
            }
          },
          {
            "SpotLightShadowMaps",
            {
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    return
                      "float calculateSpotLightShadowFactor(in vec4 lightSpacePosition, in sampler2D shadowMap, in vec3 normal, in vec3 lightDir);";
                  }
                }
            }
          },
          {
            "PointLightShadowMaps",
            {
                  {
                    ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                    {
                      return
                        "float calculatePointLightShadowFactor(in vec3 fragPos, in samplerCube shadowMap, in vec3 lightPos, in float nearPlane, in float farPlane, in vec3 normal, in vec3 lightDir);";
                    }
                  }
            }
          }
        },
      },
      {
        "preInMain",
        {
          {
            "Shape", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants) -> std::string
                {
                  return R"(int entity_id = get_entity_id();
  Entity entity = get_entity(entity_id);
  Material material = get_material(entity);)";
                }
              }
            }
          },
          {
            "SDFColor", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end() ||
                      std::find(constants.begin(), constants.end(), "DepthMap") != constants.end())
                  {
                    return "";
                  }
                  else
                  {
                    return "  FragColor = get_entity_color(entity, material);";
                  }
                }
              }
            }
          },
          {
            "Color", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end() ||
                      std::find(constants.begin(), constants.end(), "DepthMap") != constants.end())
                  {
                    return "";
                  }
                  else
                  {
                    return "  FragColor = inColor;";
                  }
                }
              }
            }
          },
          {
            "ColorTexture", {
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    std::string string("  vec4 sampled = texture(ColorTexture, inUV);\n");
//                     string += R"(  if(sampled.a < 0.8)
//     discard;
// )";
                  	if (std::find(constants.begin(), constants.end(), "TextColor") != constants.end())
                  	{
											string += "  FragColor = vec4(textColor.value.r, textColor.value.g, textColor.value.b, sampled.a * textColor.value.a);";
                  	}
                  	else
                  	{
                  		string += "  FragColor = sampled;";
                  	}
                  	return string;
                  }
                }
            }
          },
          {
            "Texture3D", {
                  {
                    ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                    {
                      return "  FragColor = texture(Texture3D, inUV);";
                    }
                  }
            }
          },
          {
            "TextureCube", {
                  {
                    ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                    {
                      std::string string = "  vec3 sampleDir = normalize(inUV);\n";
                      string += "  sampleDir = clamp(sampleDir, -0.999, 0.999);\n";
                      string += "  FragColor = texture(TextureCube, sampleDir);";
                      return string;
                    }
                  }
            }
          },
          {
            "PointLightSpaceMatrix", {
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    return std::string("  float lightDistance = length(inFragPosition.xyz - lightPos);\n") +
                      "  gl_FragDepth = lightDistance / farPlane;\n";
                  }
                }
            }
          }
        }
      },
      {
        "postInMain", {
          {
            "Fog", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  return std::string("  float distance = length(inPosition.xyz - CameraPosition.value);\n") +
                    "  float fogFactor = calculateFogFactor(distance, fogDensity.value);\n"
                    "  FragColor = mix(fogColor.value, FragColor, fogFactor);";
                }
              }
            }
          },
          {
            "Lighting", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  return std::string("  vec3 normal = normalize(inNormal);\n") +
                    "  vec3 viewDir = normalize(CameraPosition.value - inFragPosition.xyz);\n"
                    "  vec3 lightingColor = vec3(0.0);\n";
                }
              }
            }
          },
          {
            "PointLightShadowMaps", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
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
                }
              }
            }
          },
          {
            "DirectionalLightShadowMaps", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
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
                }
              }
            }
          },
          {
            "SpotLightShadowMaps", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
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
                }
              }
            }
          }
        }
      },
      {
        "postPostInMain", {
          {
            "Lighting", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  return "  FragColor = FragColor * vec4(lightingColor, 1.0);";
                }
              }
            }
          }
        }
      },
      {
        "postMain", {
          {
            "Shape", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants) -> std::string
                {
                  return R"(
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
float sphereSDF(vec3 p, float r) {
    return length(p) - r;
}
float objectSDF(vec3 p_world) {
    vec3 p_local = vec3(InverseInstanceModels.data[inID] * vec4(p_world, 1.0));
    return sphereSDF(p_local, 0.49);
}
const int MAX_STEPS = 100;
const float MIN_DIST = 0.01;
const float MAX_DIST = 10.0;
vec4 sdf_get_color_sphere(vec4 baseColor)
{
  vec3 rayOrigin = inFragPosition.xyz;
  vec3 rayDirection = normalize(rayOrigin - CameraPosition.value);
  float totalDistance = 0.0;
  vec3 currentPos = rayOrigin;
  float distanceToSurface = 0.0;
  bool hit = false; 
  for (int i = 0; i < MAX_STEPS; ++i) {
      currentPos = rayOrigin + rayDirection * totalDistance;
      distanceToSurface = objectSDF(currentPos);
      if (distanceToSurface < MIN_DIST) {
          hit = true;
          break;
      }
      totalDistance += distanceToSurface;
      if (totalDistance > MAX_DIST) {
          break;
      }
  }
  if (hit) {
    return baseColor;
      // // Calculate surface normal (using finite difference)
      // // Evaluate the SDF in world space for normal calculation
      // vec3 normal;
      // vec2 eps = vec2(0.001, 0.0);
      // normal.x = objectSDF(currentPos + eps.xyy) - objectSDF(currentPos - eps.xyy);
      // normal.y = objectSDF(currentPos + eps.yxy) - objectSDF(currentPos - eps.yxy);
      // normal.z = objectSDF(currentPos + eps.yyx) - objectSDF(currentPos - eps.yyx);
      // normal = normalize(normal);

      // // Simple diffuse lighting
      // vec3 lightDir = normalize(vec3(1.0, 1.0, 1.0)); // Example light direction
      // float diff = max(dot(normal, lightDir), 0.0);

      // // Output color (e.g., based on diffuse light)
      // vec3 objectColor = vec3(0.2, 0.7, 0.3); // Example object color

      // // Example of using the instance ID to change color per instance
      // // if (v_InstanceID % 2 == 0) {
      // //     objectColor = vec3(0.8, 0.1, 0.1); // Red for even instances
      // // } else {
      // //     objectColor = vec3(0.1, 0.1, 0.8); // Blue for odd instances
      // // }


      // FragColor = vec4(objectColor * diff, 1.0);

      // // You could also encode depth or other information here
      // // FragColor = vec4(vec3(totalDistance / MAX_DIST), 1.0); // Visualize depth
  }
  return vec4(0.0, 0.0, 0.0, 0.0);
}
vec4 get_color(in Entity entity, in Material material)
{
  vec4 baseColor = FragColor;
  switch (entity.shape_type) {
    case 3:
      return sdf_get_color_sphere(baseColor);
    default:
      return baseColor;
  }
  return baseColor;
}
int get_entity_id()
{
  return inID;
};
vec4 get_entity_color(in Entity entity, in Material material)
{
  return get_color(entity, material);
}
)";
                }
              }
            }
          },
          {
            "Fog", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  return std::string("float calculateFogFactor(in float distance, in float density) {\n") +
                    "  return exp(-density * distance);\n" +
                    "}";
                }
              }
            }
          },
          {
            "Lighting", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
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
                }
              }
            }
          },
          {
            "DirectionalLightShadowMaps", {
              {
                ++ShaderFactory::hooksCount, [](shaders::Shader& shader, const auto& constants)-> std::string
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
                }
              }
            }
          },
          {
            "SpotLightShadowMaps", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  std::string string(
                      "float calculateSpotLightShadowFactor(in vec4 lightSpacePosition, in sampler2D shadowMap, in vec3 normal, in vec3 lightDir){\n");
                  string += "  vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;\n";
                  if (shader.iRenderer->renderer != RENDERER::RENDERER_VULKAN)
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
                }
              }
            }
          },
          {
            "PointLightShadowMaps", {
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    return std::string("float calculatePointLightShadowFactor(in vec3 fragPos, in samplerCube shadowMap, in vec3 lightPos, in float nearPlane, in float farPlane, in vec3 normal, in vec3 lightDir){\n") +
                      "  vec3 lightToFrag = fragPos - lightPos;\n" +
                      "  float currentDepth = length(lightToFrag) / farPlane;\n" +
                      "  float closestDepth = texture(shadowMap, normalize(lightToFrag)).r;\n" +
                      "  float bias = 0.005;\n" +
                      "  return (currentDepth - bias) > closestDepth ? 1.0 : 0.0;\n" +
                      "}";
                  }
                }
            }
          }
        }
      }
    }
  }
};
uint32_t ShaderFactory::hooksCount = 0;
ShaderFactory::ShaderHookInfoMap ShaderFactory::shaderHookInfos;
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
uint32_t ShaderFactory::currentInLayoutIndex = 0;
uint32_t ShaderFactory::currentOutLayoutIndex = 0;
uint32_t ShaderFactory::currentBindingIndex = 0;
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
uint32_t ShaderFactory::addHook(const ShaderType& shaderType, const std::string_view hookName,
																const std::string_view runtimeConstant, const Shader::ShaderHook& hook)
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
