#include <anex/modules/gl/shaders/Shader.hpp>
#include <anex/modules/gl/shaders/ShaderFactory.hpp>
#include <anex/modules/gl/textures/Texture.hpp>
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
    GLcheck("glDeleteBuffers");
  }
  for (auto& bindingPair : uboBindings)
  {
    glDeleteBuffers(1, &std::get<1>(bindingPair.second));
    GLcheck("glDeleteBuffers");
  }
  ShaderFactory::deleteProgram(*this);
};
void Shader::bind() const
{
  glUseProgram(program);
  GLcheck("glUseProgram");
};
void Shader::unbind() const
{
  glUseProgram(0);
  GLcheck("glUseProgram");
};
void Shader::addSSBO(const std::string& name, const uint32_t &bindingIndex)
{
  GLuint ssboBufferID;
  glGenBuffers(1, &ssboBufferID);
  GLcheck("glGenBuffers");
  auto& ssboBinding = ssboBindings[name];
  std::get<0>(ssboBinding) = bindingIndex;
  std::get<1>(ssboBinding) = ssboBufferID;
};
void Shader::addUBO(const std::string &name, const uint32_t &bindingIndex)
{
  GLuint uboBufferID;
  glGenBuffers(1, &uboBufferID);
  GLcheck("glGenBuffers");
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
  GLcheck("glBindBufferBase");
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
  GLcheck("glBindBuffer");
  glBufferData(GL_SHADER_STORAGE_BUFFER, size, pointer, GL_STATIC_DRAW);
  GLcheck("glBufferData");
};
void Shader::setTexture(const std::string &name, const textures::Texture &texture, const int32_t &unit)
{
  bind();
  setUniform(name, unit);
  glActiveTexture(GL_TEXTURE0 + unit);
  texture.bind();
  unbind();
};