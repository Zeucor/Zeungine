#include <anex/modules/gl/shaders/Shader.hpp>
#include <anex/modules/gl/shaders/ShaderFactory.hpp>
using namespace anex::modules::gl::shaders;
Shader::Shader(const RuntimeConstants &constants):
	shaders(ShaderFactory::generateShaderMap(constants, *this))
{
  ShaderFactory::compileProgram(shaders, program);
};
Shader::~Shader()
{
  for (auto& bindingPair : ssboBindings)
  {
    glDeleteBuffers(1, &std::get<1>(bindingPair.second));
  }
  for (auto& bindingPair : uboBindings)
  {
    glDeleteBuffers(1, &std::get<1>(bindingPair.second));
  }
  ShaderFactory::deleteProgram(*this);
};
void Shader::use(const bool &useProgram)
{
  if (useProgram)
  {
    glUseProgram(program);
  }
  else
  {
    glUseProgram(0);
  }
};
void Shader::addSSBO(const std::string& name, const uint32_t &size, const uint32_t &bindingIndex)
{
  GLuint ssboBufferID;
  glGenBuffers(1, &ssboBufferID);
  auto& ssboBinding = ssboBindings[name];
  std::get<0>(ssboBinding) = bindingIndex;
  std::get<1>(ssboBinding) = ssboBufferID;
};
void Shader::addUBO(const std::string &name, const uint32_t &size, const uint32_t &bindingIndex)
{
  GLuint uboBufferID;
  glGenBuffers(1, &uboBufferID);
  auto& uboBinding = uboBindings[name];
  std::get<0>(uboBinding) = bindingIndex;
  std::get<1>(uboBinding) = uboBufferID;
};
void Shader::setSSBO(const std::string &name, const void *pointer, const uint32_t &size)
{
  auto ssboIter = ssboBindings.find(name.c_str());
  if (ssboIter == ssboBindings.end())
  {
    return;
  }
  auto& bindingIndex = std::get<0>(ssboIter->second);
  auto& buffer = std::get<1>(ssboIter->second);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, pointer, GL_STATIC_DRAW);
};