#include <zg/shaders/Shader.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/textures/Texture.hpp>
using namespace zg::shaders;
Shader::Shader(Window &window, const RuntimeConstants &constants, const std::vector<ShaderType> &shaderTypes):
  window(window),
	shaders(ShaderFactory::generateShaderMap(constants, *this, shaderTypes))
{
  ShaderFactory::compileProgram(*this, shaders, program);
};
Shader::~Shader()
{
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iVendorRenderer);
  for (auto& bindingPair : ssboBindings)
  {
    glRenderer.glContext->DeleteBuffers(1, &std::get<1>(bindingPair.second));
    GLcheck(glRenderer, "glDeleteBuffers");
  }
  for (auto& bindingPair : uboBindings)
  {
    glRenderer.glContext->DeleteBuffers(1, &std::get<1>(bindingPair.second));
    GLcheck(glRenderer, "glDeleteBuffers");
  }
  ShaderFactory::deleteProgram(*this);
};
void Shader::bind() const
{
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iVendorRenderer);
  glRenderer.glContext->UseProgram(program);
  GLcheck(glRenderer, "glUseProgram");
};
void Shader::unbind() const
{
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iVendorRenderer);
  glRenderer.glContext->UseProgram(0);
  GLcheck(glRenderer, "glUseProgram");
};
void Shader::addSSBO(const std::string_view name, uint32_t bindingIndex)
{
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iVendorRenderer);
  GLuint ssboBufferID;
  glRenderer.glContext->GenBuffers(1, &ssboBufferID);
  GLcheck(glRenderer, "glGenBuffers");
  auto& ssboBinding = ssboBindings[name];
  std::get<0>(ssboBinding) = bindingIndex;
  std::get<1>(ssboBinding) = ssboBufferID;
};
void Shader::addUBO(const std::string_view name, uint32_t bindingIndex)
{
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iVendorRenderer);
  GLuint uboBufferID;
  glRenderer.glContext->GenBuffers(1, &uboBufferID);
  GLcheck(glRenderer, "glGenBuffers");
  auto& uboBinding = uboBindings[name];
  std::get<0>(uboBinding) = bindingIndex;
  std::get<1>(uboBinding) = uboBufferID;
};
void Shader::setSSBO(const std::string_view name, const void *pointer, uint32_t size)
{
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iVendorRenderer);
  auto ssboIter = ssboBindings.find(name.data());
  if (ssboIter == ssboBindings.end())
  {
    return;
  }
  auto& bindingIndex = std::get<0>(ssboIter->second);
  auto& buffer = std::get<1>(ssboIter->second);
  glRenderer.glContext->BindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, buffer);
  GLcheck(glRenderer, "glBindBufferBase");
  glRenderer.glContext->BindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
  GLcheck(glRenderer, "glBindBuffer");
  glRenderer.glContext->BufferData(GL_SHADER_STORAGE_BUFFER, size, pointer, GL_STATIC_DRAW);
  GLcheck(glRenderer, "glBufferData");
};
void Shader::setTexture(const std::string_view name, const textures::Texture &texture, const int32_t &unit)
{
	auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iVendorRenderer);
  setUniform(name, unit);
  glRenderer.glContext->ActiveTexture(GL_TEXTURE0 + unit);
  texture.bind();
};