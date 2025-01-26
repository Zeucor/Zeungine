#pragma once
#ifdef USE_GL
#include "../common.hpp"
#include "../IVendorRenderer.hpp"
#include "../IPlatformWindow.hpp"
namespace zg::modules::gl
{
	struct GLRenderer : IVendorRenderer
	{
		GladGLContext *glContext = 0;
		GLRenderer();
		~GLRenderer() override;
		void init(IPlatformWindow* platformWindowPointer) override;
		void render() override;
		void destroy() override;
	};
	const bool GLcheck(GLRenderer &renderer, const char* fn, const bool egl = false);
}
#endif