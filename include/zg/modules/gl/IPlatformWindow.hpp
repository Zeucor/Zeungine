#pragma once
#include <memory>
#include <zg/glm.hpp>
namespace zg::modules::gl
{
	struct RenderWindow;
	struct IPlatformWindow
	{
		RenderWindow *renderWindowPointer = nullptr;
		virtual ~IPlatformWindow() = default;
		virtual void init(RenderWindow &renderWindow) = 0;
		virtual void postInit() = 0;
		virtual void loop() = 0;
		virtual void destroy() = 0;
		virtual void close() = 0;
		virtual void minimize() = 0;
		virtual void maximize() = 0;
		virtual void restore() = 0;
		virtual void warpPointer(glm::vec2 coords) = 0;
		virtual void setXY() = 0;
		virtual void setWidthHeight() = 0;
		virtual void mouseCapture(bool capture) = 0;
	};
	std::shared_ptr<IPlatformWindow> createPlatformWindow();
}