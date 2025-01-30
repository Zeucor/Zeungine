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
	};
	std::shared_ptr<IVendorRenderer> createVendorRenderer();
}