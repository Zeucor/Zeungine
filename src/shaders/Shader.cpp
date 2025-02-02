#include <zg/shaders/Shader.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/textures/Texture.hpp>
using namespace zg::shaders;
Shader::Shader(Window &window, const RuntimeConstants &constants, const std::vector<ShaderType> &shaderTypes):
  window(window)
{
  window.iRenderer->initShader(*this, constants, shaderTypes);
}
Shader::~Shader()
{
  window.iRenderer->destroyShader(*this);
}
void Shader::bind() const
{
  window.iRenderer->bindShader(*this);
}
void Shader::unbind() const
{
  window.iRenderer->unbindShader(*this);
}
void Shader::addSSBO(const std::string_view name, uint32_t bindingIndex)
{
  window.iRenderer->addSSBO(*this, name, bindingIndex);
}
void Shader::addUBO(const std::string_view name, uint32_t bindingIndex)
{
  window.iRenderer->addUBO(*this, name, bindingIndex);
}
void Shader::setSSBO(const std::string_view name, const void *pointer, uint32_t size)
{
  window.iRenderer->setSSBO(*this, name, pointer, size);
}
void Shader::setTexture(const std::string_view name, const textures::Texture &texture, const int32_t unit)
{
  window.iRenderer->setTexture(*this, name, texture, unit);
}