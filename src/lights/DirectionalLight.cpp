#include <iostream>
#include <zg/lights/DirectionalLight.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderManager.hpp>
#include <zg/vp/Projection.hpp>
#include <zg/vp/View.hpp>
#include <zg/Registry.hpp>
using namespace zg::lights;
DirectionalLightShadow::DirectionalLightShadow(const std::vector<size_t*>& INDEX_STACK, size_t directionalLightIndex) :
		INDEX_STACK(INDEX_STACK), directionalLightIndex(directionalLightIndex),
		iRenderer(Registry::getWindow(INDEX_STACK).iRenderer),
		texture(std::make_shared<textures::Texture>(iRenderer, glm::ivec4(8192, 8192, 1, 0), (const void*)0, textures::Texture::Depth, textures::Texture::Float, textures::Texture::FilterType::Linear, true)),
		framebuffer(std::make_shared<textures::Framebuffer>(iRenderer, std::vector<textures::Framebuffer::TextureAttachmentPair>{{texture, textures::Framebuffer::AttachmentType::Depth}}))
{
	update();
}
DirectionalLightShadow::DirectionalLightShadow(const DirectionalLightShadow& other):
	INDEX_STACK(other.INDEX_STACK),
	directionalLightIndex(other.directionalLightIndex),
	iRenderer(other.iRenderer),
	shader(other.shader),
	texture(other.texture),
	framebuffer(other.framebuffer),
	lightSpaceMatrix(other.lightSpaceMatrix),
	lookAtSet(other.lookAtSet),
	lookAt(other.lookAt)
{
	update();
}
DirectionalLightShadow& DirectionalLightShadow::operator=(const DirectionalLightShadow& other)
{
	INDEX_STACK = other.INDEX_STACK;
	directionalLightIndex = other.directionalLightIndex;
	iRenderer = other.iRenderer;
	shader = other.shader;
	texture = other.texture;
	framebuffer = other.framebuffer;
	lightSpaceMatrix = other.lightSpaceMatrix;
	lookAtSet = other.lookAtSet;
	lookAt = other.lookAt;
	update();
	return *this;
}
void DirectionalLightShadow::addShader()
{
	if (shader)
		return;
	void* data = 0;
	auto& vulkanRenderer = *dynamic_cast<VulkanRenderer*>(iRenderer);
	if (vulkanRenderer.currentFramebufferImpl)
	{
		data = vulkanRenderer.currentFramebufferImpl->renderPass;
	}
	else
	{
		data = vulkanRenderer.renderPass;
	}
	shader = shaders::ShaderManager::getShaderByConstants(
						 iRenderer, {"DepthMap", "Color", "Position", "Normal", "Model", "LightSpaceMatrix"}, data)
						 .second.get();
}
void DirectionalLightShadow::update()
{
	auto& window = Registry::getWindow(INDEX_STACK);
	auto& scene = Registry::getScene(INDEX_STACK);
	auto& directionalLight = scene.directionalLights[directionalLightIndex];
	static glm::vec2 projectionDimensions = {64, 64};
	vp::Projection projection(window, projectionDimensions, directionalLight.nearPlane, directionalLight.farPlane);
	vp::View view(directionalLight.position, directionalLight.direction, directionalLight.up, lookAtSet, lookAt);
	{
		std::unique_lock lock(view.updateMutex);
		view.updateCV.wait(lock, [&]
		{
			return !view.dirty;
		});
	}
	lightSpaceMatrix = projection.matrix * view.matrix;
}
