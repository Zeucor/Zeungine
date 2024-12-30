#include <anex/modules/gl/shaders/ShaderManager.hpp>
#include <anex/modules/gl/shaders/ShaderFactory.hpp>
#include <anex/crypto/vector.hpp>
using namespace anex::modules::gl::shaders;
std::unordered_map<uint32_t, std::shared_ptr<Shader>> ShaderManager::shaders;
std::unordered_map<std::size_t, std::pair<uint32_t, std::shared_ptr<Shader>>> ShaderManager::shadersByHash;
uint32_t ShaderManager::shaderCount = 0;
Shader& ShaderManager::getShaderByID(const uint32_t &id)
{
  auto shaderIter = shaders.find(id);
  if (shaderIter == shaders.end())
    throw std::runtime_error("Shader not found");
  return *shaderIter->second;
};
std::pair<uint32_t, std::shared_ptr<Shader>> ShaderManager::getShaderByConstants(const RuntimeConstants &constants)
{
  auto hash = crypto::hashVectorOfStrings(constants);
  auto hashIter = shadersByHash.find(hash);
  if (hashIter != shadersByHash.end())
    return hashIter->second;
  auto shaderPointer = std::make_shared<Shader>(constants);
  auto id = ++shaderCount;
  shaders[id] = shaderPointer;
  std::pair<uint32_t, std::shared_ptr<Shader>> pair(id, shaderPointer);
  shadersByHash[hash] = pair;
  return pair;
};
void ShaderManager::deleteShaders()
{
  shaderCount = 0;
  shaders.clear();
  shadersByHash.clear();
};