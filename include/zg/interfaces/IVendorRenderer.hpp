#pragma once
#include <memory>
#include <zg/glm.hpp>
#include <zg/enums/EUniformType.hpp>
namespace zg
{
	struct IPlatformWindow;
	namespace shaders
	{
		struct Shader;
	}
	namespace textures
	{
		struct Texture;
	}
	struct IVendorRenderer
	{
		IPlatformWindow* platformWindowPointer = nullptr;
		virtual ~IVendorRenderer() = default;
		virtual void init(IPlatformWindow *platformWindowPointer) = 0;
		virtual void render() = 0;
		virtual void destroy() = 0;
		virtual void clearColor(glm::vec4 color) = 0;
		virtual void clear() = 0;
		virtual void viewport(glm::ivec4 vp) = 0;
		virtual void setUniform(shaders::Shader &shader, const std::string_view name, const void *value, uint32_t size, enums::EUniformType uniformType) = 0;
		virtual void setBlock(shaders::Shader &shader, const std::string_view name, const void* pointer, size_t size) = 0;
		virtual void deleteBuffer(uint32_t id) = 0;
		virtual void bindShader(const shaders::Shader &shader) = 0;
		virtual void unbindShader(const shaders::Shader &shader) = 0;
		virtual void addSSBO(shaders::Shader &shader, const std::string_view name, uint32_t bindingIndex) = 0;
		virtual void addUBO(shaders::Shader &shader, const std::string_view name, uint32_t bindingIndex) = 0;
		virtual void setSSBO(shaders::Shader &shader, const std::string_view name, const void *pointer, size_t size) = 0;
		virtual void setTexture(shaders::Shader &shader, const std::string_view name, const textures::Texture &texture, const int32_t unit) = 0;
	};
	std::shared_ptr<IVendorRenderer> createVendorRenderer();
}