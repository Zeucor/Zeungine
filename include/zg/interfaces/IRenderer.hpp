#pragma once
#include <memory>
#include <zg/glm.hpp>
#include <zg/enums/EUniformType.hpp>
#include <zg/enums/ShaderType.hpp>
#include <zg/images/ImageLoader.hpp>
#include <map>
namespace zg
{
	struct IPlatformWindow;
	namespace shaders
	{
		struct Shader;
		using ShaderPair = std::pair<std::string, uint32_t>;
		using ShaderMap = std::map<ShaderType, ShaderPair>;
	}
	namespace textures
	{
		struct Texture;
		struct Framebuffer;
	}
	namespace vaos
	{
		struct VAO;
	}
	struct IRenderer
	{
		IPlatformWindow* platformWindowPointer = nullptr;
		virtual ~IRenderer() = default;
		virtual void init(IPlatformWindow *platformWindowPointer) = 0;
		virtual void destroy() = 0;
		virtual void clearColor(glm::vec4 color) = 0;
		virtual void clear() = 0;
		virtual void viewport(glm::ivec4 vp) const = 0;
		virtual void setUniform(shaders::Shader &shader, const std::string_view name, const void *pointer, uint32_t size, enums::EUniformType uniformType) = 0;
		virtual void setBlock(shaders::Shader &shader, const std::string_view name, const void* pointer, size_t size) = 0;
		virtual void deleteBuffer(uint32_t id) = 0;
		virtual void bindShader(const shaders::Shader &shader) = 0;
		virtual void unbindShader(const shaders::Shader &shader) = 0;
		virtual void addSSBO(shaders::Shader &shader, const std::string_view name, uint32_t bindingIndex) = 0;
		virtual void addUBO(shaders::Shader &shader, const std::string_view name, uint32_t bindingIndex) = 0;
		virtual void setSSBO(shaders::Shader &shader, const std::string_view name, const void *pointer, size_t size) = 0;
		virtual void setTexture(shaders::Shader &shader, const std::string_view name, const textures::Texture &texture, const int32_t unit) = 0;
		virtual bool compileShader(shaders::Shader &shader, shaders::ShaderType shaderType, shaders::ShaderPair &shaderPair) = 0;
		virtual bool compileProgram(shaders::Shader &shader, const shaders::ShaderMap& shaderMap) = 0;
		virtual void deleteShader(shaders::Shader &shader) = 0;
		virtual void bindFramebuffer(const textures::Framebuffer &framebuffer) const = 0;
		virtual void unbindFramebuffer(const textures::Framebuffer &framebuffer) const = 0;
		virtual void initFramebuffer(textures::Framebuffer &framebuffer) = 0;
		virtual void destroyFramebuffer(textures::Framebuffer &framebuffer) = 0;
		virtual void bindTexture(const textures::Texture &texture) = 0;
		virtual void unbindTexture(const textures::Texture &texture) = 0;
		virtual void preInitTexture(textures::Texture &texture) = 0;
		virtual void midInitTexture(const textures::Texture &texture, const std::vector<images::ImageLoader::ImagePair>& images) = 0;
		virtual void postInitTexture(const textures::Texture &texture) = 0;
		virtual void destroyTexture(textures::Texture &texture) = 0;
		virtual void updateIndicesVAO(const vaos::VAO &vao, const std::vector<uint32_t> &indices) = 0;
		virtual void updateElementsVAO(const vaos::VAO &vao, const std::string_view constant, uint8_t *elementsAsChar) = 0;
		virtual void drawVAO(const vaos::VAO &vao) = 0;
		virtual void generateVAO(vaos::VAO &vao) = 0;
		virtual void destroyVAO(vaos::VAO &vao) = 0;
	};
	std::shared_ptr<IRenderer> createVendorRenderer();
}