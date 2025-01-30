#pragma once
#ifdef USE_GL
#include "../common.hpp"
#include "../interfaces/IVendorRenderer.hpp"
#include "../interfaces/IPlatformWindow.hpp"
namespace zg
{
	struct GLRenderer : IVendorRenderer
	{
		GladGLContext *glContext = 0;
		GLRenderer();
		~GLRenderer() override;
		void init(IPlatformWindow* platformWindowPointer) override;
		void render() override;
		void destroy() override;
		void clearColor(glm::vec4 color) override;
		void clear() override;
		void viewport(glm::ivec4 vp) override;
		void setUniform(shaders::Shader &shader, const std::string_view name, const void *value, uint32_t size, enums::EUniformType uniformType) override;
	};
	const bool GLcheck(GLRenderer &renderer, const char* fn, const bool egl = false);
}
#endif