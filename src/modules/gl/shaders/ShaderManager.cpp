#include <anex/modules/gl/shaders/ShaderManager.hpp>
#include <anex/modules/gl/shaders/ShaderFactory.hpp>
#include <anex/crypto/vector.hpp>
using namespace anex::modules::gl::shaders;
Shader& ShaderManager::getShaderByID(GLWindow &window, uint32_t id)
{
  auto shaderIter = window.shaderContext->shaders.find(id);
  if (shaderIter == window.shaderContext->shaders.end())
    throw std::runtime_error("Shader not found");
  return *shaderIter->second;
};
std::pair<uint32_t, std::shared_ptr<Shader>> ShaderManager::getShaderByConstants(GLWindow &window, const RuntimeConstants &constants, const std::vector<Shader::ShaderType> &shaderTypes)
{
  auto hash = crypto::hashVector(constants);
  auto hashIter = window.shaderContext->shadersByHash.find(hash);
  if (hashIter != window.shaderContext->shadersByHash.end())
    return hashIter->second;
  auto shaderPointer = std::make_shared<Shader>(window, constants, shaderTypes);
  auto id = ++window.shaderContext->shaderCount;
  window.shaderContext->shaders[id] = shaderPointer;
  std::pair<uint32_t, std::shared_ptr<Shader>> pair(id, shaderPointer);
  window.shaderContext->shadersByHash[hash] = pair;
  return pair;
};