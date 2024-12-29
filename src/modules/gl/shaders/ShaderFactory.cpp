#include <anex/modules/gl/shaders/ShaderFactory.hpp>
#include <stdexcept>
using namespace anex::modules::gl::shaders;
ShaderFactory::ShaderHooksMap ShaderFactory::hooks = {
  {
    Shader::ShaderType::Vertex, {
      {
        "layout", {
          {
            "Color", {
              {
                ++ShaderFactory::hooksCount, []()->std::string{
                  return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) + ") in vec4 inColor;";
                }
              },
              {
                ++ShaderFactory::hooksCount, []()->std::string{
                  return "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) + ") out vec4 outColor;";
                }
              }
            }
          },
          {
            "Position", {
              {
                ++ShaderFactory::hooksCount, []()->std::string{
                  return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) + ") in vec3 inPosition;";
                }
              },
              {
                ++ShaderFactory::hooksCount,  []()->std::string{
                  return "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) + ") out vec3 outPosition;";
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
                ++ShaderFactory::hooksCount, []()->std::string{
                  return "  outColor = inColor;";
                }
              }
            }
          },
          {
            "Position", {
              {
                ++ShaderFactory::hooksCount, []()->std::string{
                  return "  gl_Position = vec4(inPosition, 1);";
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
                ++ShaderFactory::hooksCount, []()->std::string{
                  return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) + ") in vec4 inColor;";
                },
              },
              {
                ++ShaderFactory::hooksCount, []()->std::string{
                  return "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) + ") out vec4 FragColor;";
                }
              }
            }
          },
          {
            "Position", {
              {
                ++ShaderFactory::hooksCount, []()->std::string{
                  return "layout(location = " + std::to_string(ShaderFactory::currentInLayoutIndex++) + ") in vec3 inPosition;";
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
            "Color", {
              {
                ++ShaderFactory::hooksCount, []()->std::string{
                  return "  FragColor = inColor;";
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
Shader::ShaderMap ShaderFactory::generateShaderMap(const RuntimeConstants &constants)
{
  Shader::ShaderMap shaderMap;
  shaderMap[Shader::ShaderType::Vertex] = generateShader(Shader::ShaderType::Vertex, constants);
  shaderMap[Shader::ShaderType::Fragment] = generateShader(Shader::ShaderType::Fragment, constants);
  return shaderMap;
};
Shader::ShaderPair ShaderFactory::generateShader(const Shader::ShaderType &shaderType, const RuntimeConstants &constants)
{
  Shader::ShaderPair shaderPair;
  auto &shaderString = shaderPair.first;
  auto &shaderHooks = hooks[shaderType];
  shaderString += "#version 430 core\n";
  currentInLayoutIndex = 0;
  currentOutLayoutIndex = 0;
  appendHooks(shaderString, shaderHooks["layout"], constants);
  appendHooks(shaderString, shaderHooks["preMain"], constants);
  shaderString += "void main()\n{\n";
  appendHooks(shaderString, shaderHooks["preInMain"], constants);
  appendHooks(shaderString, shaderHooks["postInMain"], constants);
  shaderString += "}\n";
  appendHooks(shaderString, shaderHooks["postMain"], constants);
  if (!compileShader(shaderType, shaderPair))
  {
    throw std::runtime_error("Failed to compile fragment shader");
  }
  return shaderPair;
};
void ShaderFactory::appendHooks(std::string &shaderString, RuntimeHooksMap &runtimeHooks, const RuntimeConstants &constants)
{
  for (const std::string &constant : constants)
  {
    const auto &constantHooks = runtimeHooks[constant];
    for (auto &hook : constantHooks)
    {
      shaderString += hook.second() + "\n";
    }
  }
}
bool ShaderFactory::compileShader(const Shader::ShaderType &shaderType, Shader::ShaderPair &shaderPair)
{
  auto &shaderString = shaderPair.first;
  auto &shaderInt = shaderPair.second;
  shaderInt = glCreateShader(shaderTypes[shaderType]);
  const GLchar* source = shaderString.c_str();
  GLint lengths[] = { (GLint)shaderString.size() };
  glShaderSource(shaderInt, 1, &(source), lengths);
  glCompileShader(shaderInt);
  return checkCompileErrors(shaderInt, true, shaderNames[shaderType].c_str());
};
bool ShaderFactory::compileProgram(const Shader::ShaderMap &shaderMap, GLuint &program)
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
void ShaderFactory::deleteProgram(Shader &shader)
{
  glDeleteProgram(shader.program);
};
uint32_t ShaderFactory::addHook(const Shader::ShaderType &shaderType, const std::string &hookName, const std::string &runtimeConstant, const Shader::ShaderHook &hook)
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