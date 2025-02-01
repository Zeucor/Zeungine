#ifdef USE_VULKAN
#include <zg/Logger.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/renderers/VulkanRenderer.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/textures/TextureFactory.hpp>
#include <zg/vaos/VAOFactory.hpp>
#ifdef WINDOWS
#include <zg/windows/WIN32Window.hpp>
#endif
#ifdef LINUX
#include <zg/windows/X11Window.hpp>
#endif
#ifdef MACOS
#include <zg/windows/MacOSWindow.hpp>
#endif
using namespace zg;
VulkanRenderer::VulkanRenderer()
{
}
VulkanRenderer::~VulkanRenderer(){}
void VulkanRenderer::createContext(IPlatformWindow *platformWindowPointer)
{
	this->platformWindowPointer = platformWindowPointer;
}
void VulkanRenderer::init()
{
}
void VulkanRenderer::destroy() {}
std::shared_ptr<IRenderer> zg::createRenderer()
{
	return std::shared_ptr<IRenderer>(new VulkanRenderer());
}
void VulkanRenderer::clearColor(glm::vec4 color)
{
}
void VulkanRenderer::clear()
{
}
void VulkanRenderer::viewport(glm::ivec4 vp) const
{
}
void VulkanRenderer::setUniform(shaders::Shader& shader, const std::string_view name, const void* pointer, uint32_t size,
														enums::EUniformType uniformType)
{
}
void VulkanRenderer::setBlock(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size)
{
}
void VulkanRenderer::deleteBuffer(uint32_t id)
{
}
void VulkanRenderer::bindShader(const shaders::Shader& shader)
{
}
void VulkanRenderer::unbindShader(const shaders::Shader& shader)
{
}
void VulkanRenderer::addSSBO(shaders::Shader& shader, const std::string_view name, uint32_t bindingIndex)
{
}
void VulkanRenderer::addUBO(shaders::Shader& shader, const std::string_view name, uint32_t bindingIndex)
{
}
void VulkanRenderer::setSSBO(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size)
{
}
void VulkanRenderer::setTexture(shaders::Shader& shader, const std::string_view name, const textures::Texture& texture,
														const int32_t unit)
{
}
bool VulkanRenderer::compileShader(shaders::Shader& shader, shaders::ShaderType shaderType, shaders::ShaderPair& shaderPair)
{
	return false;
}
bool VulkanRenderer::compileProgram(shaders::Shader& shader, const shaders::ShaderMap& shaderMap)
{
	return false;
}
bool VulkanRenderer::checkCompileErrors(shaders::Shader& shader, const uint32_t& id, bool isShader, const char* shaderType)
{
	return false;
}
void VulkanRenderer::deleteShader(shaders::Shader& shader)
{
}
void VulkanRenderer::bindFramebuffer(const textures::Framebuffer& framebuffer) const
{
}
void VulkanRenderer::unbindFramebuffer(const textures::Framebuffer& framebuffer) const
{
}
void VulkanRenderer::initFramebuffer(textures::Framebuffer& framebuffer)
{
}
void VulkanRenderer::destroyFramebuffer(textures::Framebuffer& framebuffer)
{
}
void VulkanRenderer::bindTexture(const textures::Texture& texture)
{
}
void VulkanRenderer::unbindTexture(const textures::Texture& texture)
{
}
void VulkanRenderer::preInitTexture(textures::Texture& texture)
{
}
void VulkanRenderer::midInitTexture(const textures::Texture& texture,
																const std::vector<images::ImageLoader::ImagePair>& images)
{
}
void VulkanRenderer::postInitTexture(const textures::Texture& texture)
{
}
void VulkanRenderer::destroyTexture(textures::Texture& texture)
{
}
void VulkanRenderer::updateIndicesVAO(const vaos::VAO &vao, const std::vector<uint32_t>& indices)
{
}
void VulkanRenderer::updateElementsVAO(const vaos::VAO &vao, const std::string_view constant, uint8_t* elementsAsChar)
{
}
void VulkanRenderer::drawVAO(const vaos::VAO &vao)
{
}
void VulkanRenderer::generateVAO(vaos::VAO &vao)
{
}
void VulkanRenderer::destroyVAO(vaos::VAO &vao)
{
}
const bool zg::VKcheck(const VulkanRenderer& renderer, const char* fn)
{
#ifndef NDEBUG
#endif
	return true;
}
#endif
