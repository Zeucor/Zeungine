#include <zg/lights/DirectionalLight.hpp>
#include <zg/shaders/ShaderManager.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/vp/Projection.hpp>
#include <zg/vp/View.hpp>
#include <iostream>
using namespace zg::lights;
DirectionalLightShadow::DirectionalLightShadow(Window &window,
											   DirectionalLight &directionalLight) : window(window),
																					 directionalLight(directionalLight),
																					 texture(window.iRenderer, glm::ivec4(8192, 8192, 1, 0), 0, textures::Texture::Depth, textures::Texture::Float),
																					 framebuffer(window, {{&texture, textures::Framebuffer::AttachmentType::Depth}})
{
	update();
}
DirectionalLightShadow& DirectionalLightShadow::operator=(const DirectionalLightShadow& other)
{
    shader = other.shader;
    texture = other.texture;
    lightSpaceMatrix = other.lightSpaceMatrix;
    lookAtSet = other.lookAtSet;
    lookAt = other.lookAt;
	return *this;
}
void DirectionalLightShadow::addShader()
{
	if (shader)
		return;
	void* data = 0;	
	auto& vulkanRenderer = *dynamic_cast<VulkanRenderer*>(window.iRenderer);
	if (vulkanRenderer.currentFramebufferImpl)
	{
		data = vulkanRenderer.currentFramebufferImpl->renderPass;
	}
	else
	{
		data = vulkanRenderer.renderPass;
	} 
	shader = shaders::ShaderManager::getShaderByConstants(window, {"DepthMap", "Color", "Position", "Normal", "Model", "LightSpaceMatrix"}, data).second.get();
}
void DirectionalLightShadow::update()
{
	static glm::vec2 projectionDimensions = {128, 128};
	vp::Projection projection(window, projectionDimensions, directionalLight.nearPlane, directionalLight.farPlane);
	vp::View view(directionalLight.position, directionalLight.direction, directionalLight.up, lookAtSet, lookAt);
	lightSpaceMatrix = projection.matrix * view.matrix;
}