#ifdef USE_METAL
#include <zg/Logger.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/renderers/MetalRenderer.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/textures/TextureFactory.hpp>
#include <zg/vaos/VAOFactory.hpp>
#ifdef MACOS
#include <zg/windows/MacOSWindow.hpp>
#endif
using namespace zg;
MetalRenderer::MetalRenderer() {}
MetalRenderer::~MetalRenderer() {}
void MetalRenderer::createContext(IPlatformWindow* platformWindowPointer)
{
	this->platformWindowPointer = platformWindowPointer;
}
void MetalRenderer::init() {}
void MetalRenderer::destroy() {}
std::shared_ptr<IRenderer> zg::createRenderer() { return std::shared_ptr<IRenderer>(new MetalRenderer()); }
void MetalRenderer::clearColor(glm::vec4 color) {}
void MetalRenderer::clear() {}
void MetalRenderer::viewport(glm::ivec4 vp) const {}
void MetalRenderer::setUniform(shaders::Shader& shader, const std::string_view name, const void* pointer,
																uint32_t size, enums::EUniformType uniformType)
{
}
void MetalRenderer::setBlock(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size) {}
void MetalRenderer::deleteBuffer(uint32_t id) {}
void MetalRenderer::bindShader(const shaders::Shader& shader) {}
void MetalRenderer::unbindShader(const shaders::Shader& shader) {}
void MetalRenderer::addSSBO(shaders::Shader& shader, const std::string_view name, uint32_t bindingIndex) {}
void MetalRenderer::addUBO(shaders::Shader& shader, const std::string_view name, uint32_t bindingIndex) {}
void MetalRenderer::setSSBO(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size) {}
void MetalRenderer::setTexture(shaders::Shader& shader, const std::string_view name, const textures::Texture& texture,
																const int32_t unit)
{
}
bool MetalRenderer::compileShader(shaders::Shader& shader, shaders::ShaderType shaderType,
																	 shaders::ShaderPair& shaderPair)
{
	return false;
}
bool MetalRenderer::compileProgram(shaders::Shader& shader, const shaders::ShaderMap& shaderMap) { return false; }
bool MetalRenderer::checkCompileErrors(shaders::Shader& shader, const uint32_t& id, bool isShader,
																				const char* shaderType)
{
	return false;
}
void MetalRenderer::deleteShader(shaders::Shader& shader) {}
void MetalRenderer::bindFramebuffer(const textures::Framebuffer& framebuffer) const {}
void MetalRenderer::unbindFramebuffer(const textures::Framebuffer& framebuffer) const {}
void MetalRenderer::initFramebuffer(textures::Framebuffer& framebuffer) {}
void MetalRenderer::destroyFramebuffer(textures::Framebuffer& framebuffer) {}
void MetalRenderer::bindTexture(const textures::Texture& texture) {}
void MetalRenderer::unbindTexture(const textures::Texture& texture) {}
void MetalRenderer::preInitTexture(textures::Texture& texture) {}
void MetalRenderer::midInitTexture(const textures::Texture& texture,
																		const std::vector<images::ImageLoader::ImagePair>& images)
{
}
void MetalRenderer::postInitTexture(const textures::Texture& texture) {}
void MetalRenderer::destroyTexture(textures::Texture& texture) {}
void MetalRenderer::updateIndicesVAO(const vaos::VAO& vao, const std::vector<uint32_t>& indices) {}
void MetalRenderer::updateElementsVAO(const vaos::VAO& vao, const std::string_view constant, uint8_t* elementsAsChar)
{
}
void MetalRenderer::drawVAO(const vaos::VAO& vao) {}
void MetalRenderer::generateVAO(vaos::VAO& vao) {}
void MetalRenderer::destroyVAO(vaos::VAO& vao) {}
// change void* result to the result type of a Metal call?
bool zg::MTLcheck(const char* fn, void* result)
{
	if (result)
	{
		return true;
	}
	else
	{
		return false;
	}
}
#endif
