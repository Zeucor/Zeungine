#pragma once
#ifdef USE_GL
#include "../common.hpp"
#include "../interfaces/IRenderer.hpp"
#include "../interfaces/IPlatformWindow.hpp"
namespace zg
{
	struct GLRenderer : IRenderer
	{
		GladGLContext *glContext = 0;
		GLRenderer();
		~GLRenderer() override;
		void createContext(IPlatformWindow* platformWindowPointer) override;
		void init() override;
		void destroy() override;
		void clearColor(glm::vec4 color) override;
		void clear() override;
		void viewport(glm::ivec4 vp) const override;
		void setUniform(shaders::Shader &shader, const std::string_view name, const void *pointer, uint32_t size, enums::EUniformType uniformType) override;
		void setBlock(shaders::Shader &shader, const std::string_view name, const void* pointer, size_t size) override;
		void deleteBuffer(uint32_t id) override;
		void bindShader(const shaders::Shader &shader) override;
		void unbindShader(const shaders::Shader &shader) override;
		void addSSBO(shaders::Shader &shader, const std::string_view name, uint32_t bindingIndex) override;
		void addUBO(shaders::Shader &shader, const std::string_view name, uint32_t bindingIndex) override;
		void setSSBO(shaders::Shader &shader, const std::string_view name, const void *pointer, size_t size) override;
		void setTexture(shaders::Shader &shader, const std::string_view name, const textures::Texture &texture, const int32_t unit) override;
		bool compileShader(shaders::Shader &shader, shaders::ShaderType shaderType, shaders::ShaderPair &shaderPair) override;
		bool compileProgram(shaders::Shader &shader, const shaders::ShaderMap& shaderMap) override;
		bool checkCompileErrors(shaders::Shader &shader, const GLuint& id, bool isShader, const char* shaderType);
		void deleteShader(shaders::Shader &shader) override;
		void bindFramebuffer(const textures::Framebuffer &framebuffer) const override;
		void unbindFramebuffer(const textures::Framebuffer &framebuffer) const override;
		void initFramebuffer(textures::Framebuffer &framebuffer) override;
		void destroyFramebuffer(textures::Framebuffer &framebuffer) override;
		void bindTexture(const textures::Texture &texture) override;
		void unbindTexture(const textures::Texture &texture) override;
		void preInitTexture(textures::Texture &texture) override;
		void midInitTexture(const textures::Texture &texture, const std::vector<images::ImageLoader::ImagePair>& images) override;
		void postInitTexture(const textures::Texture &texture) override;
		void destroyTexture(textures::Texture &texture) override;
		void updateIndicesVAO(const vaos::VAO &vao, const std::vector<uint32_t> &indices) override;
		void updateElementsVAO(const vaos::VAO &vao, const std::string_view constant, uint8_t *elementsAsChar) override;
		void drawVAO(const vaos::VAO &vao) override;
		void generateVAO(vaos::VAO &vao) override;
		void destroyVAO(vaos::VAO &vao) override;
	};
	const bool GLcheck(const GLRenderer &renderer, const char* fn, const bool egl = false);
}
#endif