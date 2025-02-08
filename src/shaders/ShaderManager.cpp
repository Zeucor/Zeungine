#include <zg/shaders/ShaderManager.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/crypto/vector.hpp>
using namespace zg::shaders;
Shader &ShaderManager::getShaderByID(Window &window, uint32_t id)
{
  auto shaderIter = window.shaderContext->shaders.find(id);
  if (shaderIter == window.shaderContext->shaders.end())
    throw std::runtime_error("Shader not found");
  return *shaderIter->second;
}
std::pair<uint32_t, std::shared_ptr<Shader>> ShaderManager::getShaderByConstants(Window &window,
                                                                                 const RuntimeConstants &constants,
                                                                                 void *data,
                                                                                 const std::vector<ShaderType> &shaderTypes)
{
  auto hash = crypto::hashVector(constants);
  hash = crypto::combineHashes(hash, (size_t)data);
  auto hashIter = window.shaderContext->shadersByHash.find(hash);
  if (hashIter != window.shaderContext->shadersByHash.end())
    return hashIter->second;
  auto shaderPointer = std::make_shared<Shader>(hash, window, constants, shaderTypes);
  auto id = ++window.shaderContext->shaderCount;
  window.shaderContext->shaders[id] = shaderPointer;
  std::pair<uint32_t, std::shared_ptr<Shader>> pair(id, shaderPointer);
  window.shaderContext->shadersByHash[hash] = pair;
  return pair;
}