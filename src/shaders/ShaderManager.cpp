#include <zg/shaders/ShaderManager.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/crypto/vector.hpp>
#include <iostream>
using namespace zg::shaders;
Shader &ShaderManager::getShaderByID(IRenderer* iRenderer, uint32_t id)
{
  auto shaderIter = iRenderer->shaderContext->shaders.find(id);
  if (shaderIter == iRenderer->shaderContext->shaders.end())
    throw std::runtime_error("Shader not found");
  return *shaderIter->second;
}
std::pair<uint32_t, std::shared_ptr<Shader>> ShaderManager::getShaderByConstants(IRenderer* iRenderer,
                                                                                 const RuntimeConstants &constants,
                                                                                 void *data,
                                                                                 const std::vector<ShaderType> &shaderTypes)
{
  auto hash = crypto::hashVector(constants);
  hash = crypto::combineHashes(hash, (size_t)data);
  auto hashIter = iRenderer->shaderContext->shadersByHash.find(hash);
  if (hashIter != iRenderer->shaderContext->shadersByHash.end())
    return hashIter->second;
  std::cout << "Creating shader hash: " << hash <<std::endl;
  auto shaderPointer = std::make_shared<Shader>(hash, iRenderer, constants, shaderTypes);
  auto id = ++iRenderer->shaderContext->shaderCount;
  iRenderer->shaderContext->shaders[id] = shaderPointer;
  std::pair<uint32_t, std::shared_ptr<Shader>> pair(id, shaderPointer);
  iRenderer->shaderContext->shadersByHash[hash] = pair;
  return pair;
}