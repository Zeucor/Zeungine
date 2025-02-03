#include <zg/shaders/Shader.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/textures/Texture.hpp>
#include <zg/crypto/vector.hpp>
using namespace zg::shaders;
Shader::Shader(Window &window, const RuntimeConstants &constants, const std::vector<ShaderType> &shaderTypes):
  window(window),
  constants(constants),
  hash(crypto::hashVector(constants))
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
void Shader::addSSBO(ShaderType shaderType, const std::string_view name, uint32_t bindingIndex)
{
  window.iRenderer->addSSBO(*this, shaderType, name, bindingIndex);
}
void Shader::addUBO(ShaderType shaderType, const std::string_view name, uint32_t bindingIndex, uint32_t bufferSize, uint32_t descriptorCount, bool isArray)
{
  window.iRenderer->addUBO(*this, shaderType, name, bindingIndex, bufferSize, descriptorCount, isArray);
}
void Shader::addTexture(uint32_t bindingIndex, shaders::ShaderType shaderType, std::string_view textureName, uint32_t descriptorCount)
{
  window.iRenderer->addTexture(*this, bindingIndex, shaderType, textureName, descriptorCount);
}
void Shader::setSSBO(const std::string_view name, const void *pointer, uint32_t size)
{
  window.iRenderer->setSSBO(*this, name, pointer, size);
}
void Shader::setTexture(const std::string_view name, const textures::Texture &texture, const int32_t unit)
{
  window.iRenderer->setTexture(*this, name, texture, unit);
}