#include <anex/modules/gl/shaders/ShaderFactory.hpp>
#include <anex/modules/gl/lights/Lights.hpp>
#include <anex/IWindow.hpp>
#include <stdexcept>
#include <glm/fwd.hpp>
using namespace anex::modules::gl::shaders;
ShaderFactory::ShaderHooksMap ShaderFactory::hooks = {
  {
    Shader::ShaderType::Vertex, {
      {
        "layout", {
          {
            "Color", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {

                  return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                    ") in vec4 inColor;";
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
                  return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                    ") in vec3 inPosition;";
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
                    return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                      ") in vec2 inUV;";
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
                      return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                        ") in vec3 inUV;";
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
            "View",
            {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  auto bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addUBO("View", bindingIndex);
                  return "layout(binding = " + std::to_string(bindingIndex) + ") uniform View {\n" +
                    "  mat4 matrix;\n" +
                    "} view;";
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
                  shader.addUBO("Projection", bindingIndex);
                  return "layout(binding = " + std::to_string(bindingIndex) + ") uniform Projection {\n" +
                    "  mat4 matrix;\n" +
                    "} projection;";
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
                  shader.addUBO("Model", bindingIndex);
                  return "layout(binding = " + std::to_string(bindingIndex) + ") uniform Model {\n" +
                    "  mat4 matrix;\n" +
                    "} model;";
                }
              }
            }
          },
          {
            "Normal", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) +
                    ") in vec3 inNormal;";
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
                  shader.addUBO("LightSpaceMatrix", bindingIndex);
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
                  shader.addUBO("DirectionalLightSpaceMatrices", bindingIndex);
                  string += "layout(binding = " + std::to_string(bindingIndex) +
                    ") uniform DirectionalLightSpaceMatrices {\n" +
                    " mat4 matrix[4];\n" +
                    "} directionalLightSpaceMatrices;\n";
                  string += "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex) +
                    ") out vec4 outDirectionalLightSpacePositions[4];\n";
                  ShaderFactory::currentOutLayoutIndex += 4;
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addUBO("SpotLightSpaceMatrices", bindingIndex);
                  string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform SpotLightSpaceMatrices {\n"
                    +
                    "  mat4 matrix[4];\n" +
                    "} spotLightSpaceMatrices;\n";
                  string += "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex) +
                    ") out vec4 outSpotLightSpacePositions[4];";
                  ShaderFactory::currentOutLayoutIndex += 4;
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
            "Color", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end())
                  {
                    return "";
                  }
                  else
                  {
                    return "  outColor = inColor;";
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
                    string += "projection.matrix * ";
                  }
                  if (std::find(constants.begin(), constants.end(), "View") != constants.end())
                  {
                    string += "view.matrix * ";
                  }
                  if (std::find(constants.begin(), constants.end(), "Model") != constants.end())
                  {
                    string += "model.matrix * ";
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
                  std::string string("  for (uint i = 0; i < 4; ++i){\n");
                  string += "    outDirectionalLightSpacePositions[i] = ";
                  string += "directionalLightSpaceMatrices.matrix[i] * ";
                  if (std::find(constants.begin(), constants.end(), "Model") != constants.end())
                  {
                    string += "model.matrix * ";
                  }
                  string += "vec4(inPosition, 1);\n";
                  string += "    outSpotLightSpacePositions[i] = ";
                  string += "spotLightSpaceMatrices.matrix[i] * ";
                  if (std::find(constants.begin(), constants.end(), "Model") != constants.end())
                  {
                    string += "model.matrix * ";
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
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end())
                  {
                    return string;
                  }
                  string += "  outFragPosition = model.matrix * vec4(inPosition, 1.0);\n";
                  string += "  outNormal = mat3(transpose(inverse(model.matrix))) * inNormal;";
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
                      return "  outUV = inUV;";
                    }
                  }
            }
          }
        }
      }
    }
  },
  {
    Shader::ShaderType::Geometry, {
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
                    shader.addUBO("PointLightSpaceMatrix", bindingIndex);
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
    Shader::ShaderType::Fragment, {
      {
        "layout",
        {
          {
            "Color", {
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
                      ") in vec4 inColor;";
                  }
                },
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
                  shader.addUBO("CameraPosition", bindingIndex);
                  return "layout(binding = " + std::to_string(bindingIndex) + ") uniform CameraPosition {\n" +
                    " vec3 value;\n" +
                    "} cameraPosition;";
                }
              }
            }
          },
          {
            "Fog", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  return "uniform float fogDensity;\nuniform vec4 fogColor;";
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
                    "};\n"
                    "struct DirectionalLight{\n"
                    "  vec3 position;\n"
                    "  vec3 direction;\n"
                    "  vec3 color;\n"
                    "  float intensity;\n"
                    "  float nearPlane;\n"
                    "  float farPlane;\n"
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
                    "};\n";
                  auto bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO("PointLights", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer PointLightBuffer {\n"
                    +
                    " PointLight pointLights[];\n" +
                    "};\n";
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO("DirectionalLights", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) +
                    ") buffer DirectionalLightBuffer {\n" +
                    " DirectionalLight directionalLights[];\n" +
                    "};\n";
                  bindingIndex = ShaderFactory::currentBindingIndex++;
                  shader.addSSBO("SpotLights", bindingIndex);
                  string += "layout(std430, binding = " + std::to_string(bindingIndex) + ") buffer SpotLightBuffer {\n"
                    +
                    " SpotLight spotLights[];\n" +
                    "};";
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
                  std::string string("layout(binding = " + std::to_string(ShaderFactory::currentBindingIndex) + ") uniform sampler2D directionalLightSamplers[4];");
                  currentBindingIndex += 4;
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
                  std::string string("layout(binding = " + std::to_string(ShaderFactory::currentBindingIndex) + ") uniform sampler2D spotLightSamplers[4];");
                  currentBindingIndex += 4;
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
                    std::string string("layout(binding = " + std::to_string(ShaderFactory::currentBindingIndex) + ") uniform samplerCube pointLightSamplers[4];");
                    currentBindingIndex += 4;
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
                    ") in vec4 inDirectionalLightSpacePositions[4];\n");
                  ShaderFactory::currentInLayoutIndex += 4;
                  string += "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex) +
                    ") in vec4 inSpotLightSpacePositions[4];";
                  ShaderFactory::currentInLayoutIndex += 4;
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
                    return std::string("uniform vec3 lightPos;\nuniform float nearPlane;\nuniform float farPlane;\n");
                  }
                }
            }
          },
          {
            "Texture2D", {
                    {
                      ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                      {
                        std::string string("layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                          ") out vec4 FragColor;\n");
                        string += "layout(binding = " + std::to_string(ShaderFactory::currentBindingIndex++) + ") uniform sampler2D Texture2D;";
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
                        string += "layout(binding = " + std::to_string(ShaderFactory::currentBindingIndex++) + ") uniform sampler3D Texture3D;";
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
                      string += "layout(binding = " + std::to_string(ShaderFactory::currentBindingIndex++) + ") uniform samplerCube TextureCube;";
                      return string;
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
                    "float calculateDirectionalLightShadowFactor(in vec4 lightSpacePosition, in sampler2D shadowMap, in vec3 normal, in vec3 lightDir);";
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
            "Color", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  if (std::find(constants.begin(), constants.end(), "PointLightSpaceMatrix") != constants.end())
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
            "Texture2D", {
                {
                  ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                  {
                    return "  FragColor = texture(Texture2D, inUV);";
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
                      return "  FragColor = texture(TextureCube, inUV);";
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
                  return std::string("  float distance = length(inPosition.xyz - cameraPosition.value);\n") +
                    "  float fogFactor = calculateFogFactor(distance, fogDensity);\n"
                    "  FragColor = mix(fogColor, FragColor, fogFactor);";
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
                    "  vec3 viewDir = normalize(cameraPosition.value - inFragPosition.xyz);\n"
                    "  vec3 lightingColor = vec3(0.0);\n"
                    "  uint pointLightCount = pointLights.length();\n"
                    "  uint directionalLightCount = directionalLights.length();\n"
                    "  uint spotLightCount = spotLights.length();\n"
                    "  for (uint i = 0; i < pointLightCount; ++i){\n"
                    "    vec3 lightDir = normalize(pointLights[i].position - inFragPosition.xyz);\n"
                    "    float shadowFactor = calculatePointLightShadowFactor(inFragPosition.xyz, pointLightSamplers[i], pointLights[i].position, pointLights[i].nearPlane, pointLights[i].farPlane, lightDir, normal);\n"
                    "    lightingColor += calculatePointLight(pointLights[i], inFragPosition.xyz, normal, viewDir, shadowFactor, lightDir);\n"
                    "  }\n"
                    "  for (uint i = 0; i < directionalLightCount; ++i){\n" +
                    "    vec3 lightDir = normalize(-directionalLights[i].direction);\n" +
                    "    float shadowFactor = calculateDirectionalLightShadowFactor(inDirectionalLightSpacePositions[i], directionalLightSamplers[i], normal, lightDir);\n"
                    "    lightingColor += calculateDirectionalLight(directionalLights[i], normal, viewDir, shadowFactor, lightDir);\n"
                    +
                    "  }\n"
                    "  for (uint i = 0; i < spotLightCount; ++i){\n"
                    "    vec3 lightDir = normalize(spotLights[i].position - inFragPosition.xyz);\n"
                    "    float shadowFactor = calculateSpotLightShadowFactor(inSpotLightSpacePositions[i], spotLightSamplers[i], normal, lightDir);\n"
                    +
                    "    lightingColor += calculateSpotLight(spotLights[i], inFragPosition.xyz, normal, viewDir, shadowFactor, lightDir);\n"
                    "  }\n"
                    "  FragColor = FragColor * vec4(lightingColor, 1.0);";
                }
              }
            }
          }
        }
      },
      {
        "postMain", {
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
                    "  vec3 ambient = 0.1 * light.color;\n" +
                    "  vec3 diffuse = diff * light.color * light.intensity * attenuation * (1.0 - shadowFactor);\n" +
                    "  vec3 specular = spec * light.color * light.intensity * attenuation * (1.0 - shadowFactor);\n" +
                    "  return (ambient + diffuse + specular);\n" +
                    "}\n" +
                    "vec3 calculateDirectionalLight(in DirectionalLight light, in vec3 normal, in vec3 viewDir, in float shadowFactor, in vec3 lightDir){\n"
                    +
                    "  float diff = max(dot(normal, lightDir), 0.0);\n" +
                    "  vec3 reflectDir = reflect(-lightDir, normal);\n" +
                    "  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);\n" +
                    "  vec3 ambient = 0.1 * light.color;\n" +
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
                    "  vec3 ambient = 0.1 * light.color;\n" +
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
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  return std::string(
                      "float calculateDirectionalLightShadowFactor(in vec4 lightSpacePosition, in sampler2D shadowMap, in vec3 normal, in vec3 lightDir){\n")
                    +
                    "  vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;\n" +
                    "  projCoords = projCoords * 0.5 + 0.5;\n" +
                    "  float closestDepth = texture(shadowMap, projCoords.xy).r;\n" +
                    "  float currentDepth = projCoords.z;\n" +
                    "  float bias = max(0.00001 * (1.0 - dot(normal, lightDir)), 0.00001);\n" +
                    "  float shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;\n" +
                    "  return shadow;\n" +
                    "}";
                }
              }
            }
          },
          {
            "SpotLightShadowMaps", {
              {
                ++ShaderFactory::hooksCount, [](auto& shader, const auto& constants)-> std::string
                {
                  return std::string(
                      "float calculateSpotLightShadowFactor(in vec4 lightSpacePosition, in sampler2D shadowMap, in vec3 normal, in vec3 lightDir){\n") +
                    "  vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;\n" +
                    "  projCoords = projCoords * 0.5 + 0.5;\n" +
                    "  float closestDepth = texture(shadowMap, projCoords.xy).r;\n" +
                    "  float currentDepth = projCoords.z;\n" +
                    "  float bias = max(0.00001 * (1.0 - dot(normal, lightDir)), 0.00001);\n" +
                    "  float shadow = (currentDepth - bias) > closestDepth ? 1.0 : 0.0;\n" +
                    "  return shadow;\n" +
                    "}";
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
  {Shader::ShaderType::Vertex, GL_VERTEX_SHADER},
  {Shader::ShaderType::Geometry, GL_GEOMETRY_SHADER},
  {Shader::ShaderType::Fragment, GL_FRAGMENT_SHADER},
  {Shader::ShaderType::TessellationControl, GL_TESS_CONTROL_SHADER},
  {Shader::ShaderType::TessellationEvaluation, GL_TESS_EVALUATION_SHADER},
  {Shader::ShaderType::Compute, GL_COMPUTE_SHADER}
};
ShaderFactory::ShaderNameMap ShaderFactory::shaderNames = {
  {Shader::ShaderType::Vertex, "VertexShader"},
  {Shader::ShaderType::Geometry, "GeometryShader"},
  {Shader::ShaderType::Fragment, "FragmentShader"},
  {Shader::ShaderType::TessellationControl, "TessellationControlShader"},
  {Shader::ShaderType::TessellationEvaluation, "TessellationEvaluationShader"},
  {Shader::ShaderType::Compute, "ComputeShader"}
};
uint32_t ShaderFactory::currentInLayoutIndex = 0;
uint32_t ShaderFactory::currentOutLayoutIndex = 0;
uint32_t ShaderFactory::currentBindingIndex = 0;

Shader::ShaderMap ShaderFactory::generateShaderMap(const RuntimeConstants& constants, Shader& shader, const std::vector<Shader::ShaderType> &shaderTypes)
{
  Shader::ShaderMap shaderMap;
  for (auto &shaderType : shaderTypes)
  {
    shaderMap[shaderType] = generateShader(shaderType, constants, shader);
  }
  return shaderMap;
};

Shader::ShaderPair ShaderFactory::generateShader(const Shader::ShaderType& shaderType,
                                                 const RuntimeConstants& constants,
                                                 Shader& shader)
{
  Shader::ShaderPair shaderPair;
  auto& shaderString = shaderPair.first;
  auto& shaderHooks = hooks[shaderType];
  shaderString += "#version 430 core\n";
  currentInLayoutIndex = 0;
  currentOutLayoutIndex = 0;
  appendHooks(shaderString, shaderHooks["preLayout"], constants, shader);
  appendHooks(shaderString, shaderHooks["layout"], constants, shader);
  appendHooks(shaderString, shaderHooks["preMain"], constants, shader);
  shaderString += "void main()\n{\n";
  appendHooks(shaderString, shaderHooks["preInMain"], constants, shader);
  appendHooks(shaderString, shaderHooks["postInMain"], constants, shader);
  shaderString += "}\n";
  appendHooks(shaderString, shaderHooks["postMain"], constants, shader);
  if (!compileShader(shaderType, shaderPair))
  {
    throw std::runtime_error("Failed to compile fragment shader");
  }
  return shaderPair;
};

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

bool ShaderFactory::compileShader(const Shader::ShaderType& shaderType, Shader::ShaderPair& shaderPair)
{
  auto& shaderString = shaderPair.first;
  auto& shaderInt = shaderPair.second;
  shaderInt = glCreateShader(shaderTypes[shaderType]);
  const GLchar* source = shaderString.c_str();
  GLint lengths[] = {(GLint)shaderString.size()};
  glShaderSource(shaderInt, 1, &(source), lengths);
  glCompileShader(shaderInt);
  return checkCompileErrors(shaderInt, true, shaderNames[shaderType].data());
};

bool ShaderFactory::compileProgram(const Shader::ShaderMap& shaderMap, GLuint& program)
{
  program = glCreateProgram();
  for (const auto& shaderMapPair : shaderMap)
  {
    glAttachShader(program, shaderMapPair.second.second);
  }
  glLinkProgram(program);
  for (const auto& shaderMapPair : shaderMap)
  {
    glDeleteShader(shaderMapPair.second.second);
  }
  return checkCompileErrors(program, false, "Program");
};

const bool ShaderFactory::checkCompileErrors(const GLuint& id, const bool& isShader, const char* shaderType)
{
  GLint success = 0;
  if (isShader)
  {
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
      GLint infoLogLength;
      glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
      GLchar* infoLog = new GLchar[infoLogLength + 1];
      glGetShaderInfoLog(id, infoLogLength, 0, infoLog);
      printf("SHADER_COMPILATION_ERROR(%s):\n%s\n", shaderType, infoLog);
      delete[] infoLog;
      return false;
    }
  }
  else
  {
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success)
    {
      GLint infoLogLength;
      glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
      GLchar* infoLog = new GLchar[infoLogLength + 1];
      glGetProgramInfoLog(id, infoLogLength, 0, infoLog);
      printf("PROGRAM_LINKING_ERROR(%s):\n%s\n", shaderType, infoLog);
      delete[] infoLog;
      return false;
    }
  }
  return true;
};

void ShaderFactory::deleteProgram(Shader& shader)
{
  glDeleteProgram(shader.program);
};

uint32_t ShaderFactory::addHook(const Shader::ShaderType& shaderType, const std::string_view& hookName,
                                const std::string_view& runtimeConstant, const Shader::ShaderHook& hook)
{
  auto id = ++hooksCount;
  hooks[shaderType][hookName][runtimeConstant].emplace(id, hook);
  shaderHookInfos[id] = {shaderType, hookName, runtimeConstant};
  return id;
};

void ShaderFactory::deleteHook(const uint32_t& id)
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
};
