#pragma once
#ifdef MACOS
#include "../Window.hpp"
namespace zg
{
	struct MacOSWindow : IPlatformWindow
	{
		void *nsWindow = 0;
		void *nsView;
#ifdef USE_GL
		void *glContext = 0;
#elif defined(USE_VULKAN)
		void* bitmap = 0;
		void* nsImage = 0;
		void* nsImageView = 0;
#endif
		void init(Window& window) override;
		void renderInit();
		void postInit() override;
		bool pollMessages() override;
		void swapBuffers() override;
		void destroy() override;
		void close() override;
		void minimize() override;
		void maximize() override;
		void restore() override;
		void warpPointer(glm::vec2 coords) override;
		void setXY() override;
		void setWidthHeight() override;
		void mouseCapture(bool capture) override;
	};
}
#endif