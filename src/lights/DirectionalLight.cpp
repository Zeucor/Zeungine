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
		iRenderer(Registry::GetSingleton().getWindow(INDEX_STACK).iRenderer),
		texture(std::make_shared<textures::Texture>(iRenderer, glm::ivec4(8192, 8192, 1, 0), (const void*)0, textures::Texture::Depth, textures::Texture::Float, textures::Texture::FilterType::Linear, true, textures::Texture::Multisampling::x1, textures::Texture::AddressMode::ClampToEdge)),
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
zg::shaders::Shader* DirectionalLightShadow::addShader()
{
	if (shader)
		return shader;
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
						 iRenderer, {"Viewport", "Time", "DepthMap", "Shape", "Color", "Position", "Normal", "View", "Projection", "Model"})
						 .second.get();
	return shader;
}
void DirectionalLightShadow::update()
{
	auto& window = Registry::GetSingleton().getWindow(INDEX_STACK);
	auto& scene = Registry::GetSingleton().getScene(INDEX_STACK);
	auto& directionalLight = scene.directionalLights[directionalLightIndex];
	static glm::vec2 projectionDimensions = {64, 64};
	vp::Projection _projection(window, projectionDimensions, directionalLight.nearPlane, directionalLight.farPlane);
	vp::View _view(directionalLight.position, directionalLight.direction, directionalLight.up, lookAtSet, lookAt);
	{
		std::unique_lock lock(_view.updateMutex);
		_view.updateCV.wait(lock, [&]
		{
			return !_view.dirty;
		});
	}
	projection = _projection.matrix;
	inverseProjection = glm::inverse(projection);
	view = _view.matrix;
	inverseView = glm::inverse(view);
	lightSpaceMatrix = projection * view;
}
