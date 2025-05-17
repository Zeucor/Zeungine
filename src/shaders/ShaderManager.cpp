#include <zg/shaders/ShaderManager.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
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
                                                                                 const std::vector<ShaderType> &shaderTypes)
{
  auto hash = ShaderManager::hashConstants(constants, iRenderer);
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
size_t ShaderManager::hashConstants(const RuntimeConstants& constants, IRenderer* iRenderer)
{
  auto hash = crypto::hashVector(constants);
	size_t render_pass = 0;
	auto& vulkanRenderer = *dynamic_cast<VulkanRenderer*>(iRenderer);
	if (vulkanRenderer.currentFramebufferImpl)
	{
		render_pass = size_t(vulkanRenderer.currentFramebufferImpl->renderPass);
	}
	else
	{
		render_pass = size_t(vulkanRenderer.renderPass);
	}
  return (hash << 3) ^ (render_pass << 1);
}