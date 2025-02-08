#pragma once
#include <map>
#include <memory>
#include <zg/enums/EUniformType.hpp>
#include <zg/enums/ShaderType.hpp>
#include <zg/glm.hpp>
#include <zg/images/ImageLoader.hpp>
#include <zg/shaders/RuntimeConstants.hpp>
namespace zg
{
	struct IPlatformWindow;
	namespace shaders
	{
		struct Shader;
#if defined(USE_GL) || defined(USE_EGL)
		using PShaderType = uint32_t;
#elif defined(USE_VULKAN)
		typedef void *PShaderType;
#endif
		using ShaderPair = std::pair<std::string, PShaderType>;
		using ShaderMap = std::map<ShaderType, ShaderPair>;
		inline static std::unordered_map<ShaderType, std::string> stageShaderNames =
			{
				{ShaderType::Vertex, "vertex"},
				{ShaderType::Geometry, "geometry"},
				{ShaderType::Fragment, "fragment"},
				{ShaderType::Compute, "compute"}};
	} // namespace shaders
	namespace textures
	{
		struct Texture;
		struct Framebuffer;
	} // namespace textures
	namespace vaos
	{
		struct VAO;
	}
	enum RENDERER
	{
		RENDERER_GL,
		RENDERER_EGL,
		RENDERER_VULKAN,
		RENDERER_METAL,
		RENDERER_DIRECTX
	};
	struct Entity;
	struct IRenderer
	{
		IPlatformWindow *platformWindowPointer = nullptr;
		RENDERER renderer;
		virtual ~IRenderer() = default;
		virtual void init() = 0;
		virtual void createContext(IPlatformWindow *platformWindowPointer) = 0;
		virtual void destroy() = 0;
		virtual void preBeginRenderPass() = 0;
		virtual void beginRenderPass() = 0;
		virtual void postRenderPass() = 0;
		virtual void clearColor(glm::vec4 color) = 0;
		virtual void clear() = 0;
		virtual void viewport(glm::ivec4 vp) const = 0;
		virtual void initShader(shaders::Shader &shader, const shaders::RuntimeConstants &constants,
								const std::vector<shaders::ShaderType> &shaderTypes) = 0;
		virtual void setUniform(shaders::Shader &shader, vaos::VAO &vao, const std::string_view name, const void *pointer, uint32_t size,
								enums::EUniformType uniformType) = 0;
		virtual void setBlock(shaders::Shader &shader, vaos::VAO &vao, const std::string_view name, const void *pointer, size_t size) = 0;
		virtual void deleteBuffer(uint32_t id) = 0;
		virtual void bindShader(shaders::Shader &shader, Entity &entity) = 0;
		virtual void unbindShader(shaders::Shader &shader) = 0;
		virtual void addSSBO(shaders::Shader &shader, shaders::ShaderType shaderType, const std::string_view name, uint32_t bindingIndex) = 0;
		virtual void addUBO(shaders::Shader &shader, shaders::ShaderType shaderType, const std::string_view name, uint32_t bindingIndex, uint32_t bufferSize, uint32_t descriptorCount = 1, bool isArray = false) = 0;
		virtual void addTexture(shaders::Shader &shader, uint32_t bindingIndex, shaders::ShaderType shaderType, std::string_view textureName, uint32_t descriptorCount) = 0;
		virtual void setSSBO(shaders::Shader &shader, vaos::VAO &vao, const std::string_view name, const void *pointer, size_t size) = 0;
		virtual void setTexture(shaders::Shader &shader, vaos::VAO &vao, const std::string_view name, const textures::Texture &texture,
								const int32_t unit) = 0;
		virtual bool compileShader(shaders::Shader &shader, shaders::ShaderType shaderType,
								   shaders::ShaderPair &shaderPair) = 0;
		virtual bool compileProgram(shaders::Shader &shader) = 0;
		virtual void deleteShader(shaders::Shader &shader) = 0;
		virtual void destroyShader(shaders::Shader &shader) = 0;
		virtual void bindFramebuffer(const textures::Framebuffer &framebuffer) = 0;
		virtual void unbindFramebuffer(const textures::Framebuffer &framebuffer) = 0;
		virtual void initFramebuffer(textures::Framebuffer &framebuffer) = 0;
		virtual void destroyFramebuffer(textures::Framebuffer &framebuffer) = 0;
		virtual void bindTexture(const textures::Texture &texture) = 0;
		virtual void unbindTexture(const textures::Texture &texture) = 0;
		virtual void preInitTexture(textures::Texture &texture) = 0;
		virtual void midInitTexture(const textures::Texture &texture,
									const std::vector<images::ImageLoader::ImagePair> &images) = 0;
		virtual void postInitTexture(const textures::Texture &texture) = 0;
		virtual void destroyTexture(textures::Texture &texture) = 0;
		virtual void updateIndicesVAO(const vaos::VAO &vao, const std::vector<uint32_t> &indices) = 0;
		virtual void updateElementsVAO(const vaos::VAO &vao, const std::string_view constant, uint8_t *elementsAsChar) = 0;
		virtual void drawVAO(const vaos::VAO &vao) = 0;
		virtual void generateVAO(vaos::VAO &vao) = 0;
		virtual void destroyVAO(vaos::VAO &vao) = 0;
		virtual void ensureEntity(shaders::Shader &shader, vaos::VAO &vao) = 0;
		virtual void swapBuffers() = 0;
	};
	IRenderer *createRenderer();
} // namespace zg