#include <zg/modules/gl/shaders/Shader.hpp>
#include <zg/modules/gl/shaders/ShaderFactory.hpp>
#include <zg/modules/gl/textures/Texture.hpp>
using namespace zg::modules::gl::shaders;
Shader::Shader(RenderWindow &window, const RuntimeConstants &constants, const std::vector<ShaderType> &shaderTypes):
  window(window),
	shaders(ShaderFactory::generateShaderMap(constants, *this, shaderTypes))
{
  ShaderFactory::compileProgram(*this, shaders, program);
};
Shader::~Shader()
{
  for (auto& bindingPair : ssboBindings)
  {
    window.glContext->DeleteBuffers(1, &std::get<1>(bindingPair.second));
    GLcheck(window, "glDeleteBuffers");
  }
  for (auto& bindingPair : uboBindings)
  {
    window.glContext->DeleteBuffers(1, &std::get<1>(bindingPair.second));
    GLcheck(window, "glDeleteBuffers");
  }
  ShaderFactory::deleteProgram(*this);
};
void Shader::bind() const
{
  window.glContext->UseProgram(program);
  GLcheck(window, "glUseProgram");
};
void Shader::unbind() const
{
  window.glContext->UseProgram(0);
  GLcheck(window, "glUseProgram");
};
void Shader::addSSBO(const std::string_view name, uint32_t bindingIndex)
{
  GLuint ssboBufferID;
  window.glContext->GenBuffers(1, &ssboBufferID);
  GLcheck(window, "glGenBuffers");
  auto& ssboBinding = ssboBindings[name];
  std::get<0>(ssboBinding) = bindingIndex;
  std::get<1>(ssboBinding) = ssboBufferID;
};
void Shader::addUBO(const std::string_view name, uint32_t bindingIndex)
{
  GLuint uboBufferID;
  window.glContext->GenBuffers(1, &uboBufferID);
  GLcheck(window, "glGenBuffers");
  auto& uboBinding = uboBindings[name];
  std::get<0>(uboBinding) = bindingIndex;
  std::get<1>(uboBinding) = uboBufferID;
};
void Shader::setSSBO(const std::string_view name, const void *pointer, uint32_t size)
{
  auto ssboIter = ssboBindings.find(name.data());
  if (ssboIter == ssboBindings.end())
  {
    return;
  }
  auto& bindingIndex = std::get<0>(ssboIter->second);
  auto& buffer = std::get<1>(ssboIter->second);
  window.glContext->BindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, buffer);
  GLcheck(window, "glBindBufferBase");
  window.glContext->BindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
  GLcheck(window, "glBindBuffer");
  window.glContext->BufferData(GL_SHADER_STORAGE_BUFFER, size, pointer, GL_STATIC_DRAW);
  GLcheck(window, "glBufferData");
};
void Shader::setTexture(const std::string_view name, const textures::Texture &texture, const int32_t &unit)
{
  setUniform(name, unit);
  window.glContext->ActiveTexture(GL_TEXTURE0 + unit);
  texture.bind();
};