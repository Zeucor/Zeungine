#include <iostream>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/Shader.hpp>
#include <zg/shaders/ShaderManager.hpp>
#include <zg/vaos/VAO.hpp>
#include <zg/vaos/VAOFactory.hpp>
#include <zg/Registry.hpp>
using namespace zg;
using namespace zg::vaos;
VAO::VAO(const std::vector<size_t*>& VAO_INDEX_STACK, const RuntimeConstants& constants, uint32_t indiceCount, uint32_t vertexCount) :
		constants(constants), indiceCount(indiceCount), vertexCount(vertexCount), stride(VAOFactory::getStride(constants)),
		VAO_INDEX_STACK(VAO_INDEX_STACK),
		vaoIRenderer(Registry::getWindow(VAO_INDEX_STACK).iRenderer)
{
	VAOFactory::generate(*this);
}
VAO::VAO(const VAO& other):
	constants(other.constants),
	indiceCount(other.indiceCount),
	vertexCount(other.vertexCount),
	stride(other.stride),
	VAO_INDEX_STACK(other.VAO_INDEX_STACK),
	vaoIRenderer(Registry::getWindow(VAO_INDEX_STACK).iRenderer)
{
	VAOFactory::copy(*this, other);
}
VAO::VAO() {};
VAO& VAO::operator=(const VAO& other)
{
	constants = other.constants;
	indiceCount = other.indiceCount;
	vertexCount = other.vertexCount;
	stride = other.stride;
	VAO_INDEX_STACK = other.VAO_INDEX_STACK;
	vaoIRenderer = Registry::getWindow(VAO_INDEX_STACK).iRenderer;
	VAOFactory::copy(*this, other);
	return *this;
}
VAO::~VAO() { VAOFactory::destroy(*this); }
void VAO::updateIndices(const std::vector<uint32_t>& indices)
{
	vaoIRenderer->updateIndicesVAO(*this, indices);
}
template <typename T>
void VAO::updateElements(const std::string_view constant, const std::vector<T>& elements) const
{
	auto elementsAsChar = (uint8_t*)elements.data();
	vaoIRenderer->updateElementsVAO(*this, constant, elementsAsChar);
}
template void VAO::updateElements<glm::vec2>(const std::string_view, const std::vector<glm::vec2>&) const;
template void VAO::updateElements<glm::vec3>(const std::string_view, const std::vector<glm::vec3>&) const;
template void VAO::updateElements<glm::vec4>(const std::string_view, const std::vector<glm::vec4>&) const;
void VAO::drawVAO() const
{
	vaoIRenderer->drawVAO(*this);
}
void* VAO::getShaderUHash(IRenderer* iRenderer)
{
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
	return data;
}
size_t VAO::getVAOuHash() const
{
	void* data = 0;
	auto& vulkanRenderer = *dynamic_cast<VulkanRenderer*>(vaoIRenderer);
	if (vulkanRenderer.currentFramebufferImpl)
	{
		data = vulkanRenderer.currentFramebufferImpl->renderPass;
	}
	else
	{
		data = vulkanRenderer.renderPass;
	}
	return (size_t)data ^ uid;
}
bool VAO::isEnsured()
{
	return ensuredBools[getVAOuHash()];
}
void VAO::setEnsured()
{
	ensuredBools[getVAOuHash()] = true;
}
zg::shaders::Shader* VAO::addShader(zg::shaders::Shader* setShader)
{
	auto data = getShaderUHash(vaoIRenderer);
	auto& shader = shaders[data];
	if (shader)
		return shader;
	if (setShader)
	{
		shader = setShader;
	}
	else
	{
		shader = zg::shaders::ShaderManager::getShaderByConstants(vaoIRenderer, constants, data).second.get();
	}
	return shader;
}
