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
	};
	const bool GLcheck(GLRenderer &renderer, const char* fn, const bool egl = false);
}
#endif