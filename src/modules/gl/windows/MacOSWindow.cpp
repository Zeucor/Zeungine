#ifdef MACOS
#include <zg/modules/gl/windows/MacOSWindow.hpp>
#include <zg/modules/gl/entities/Plane.hpp>
#include <iostream>
using namespace zg::modules::gl;
void MacOSWindow::init(RenderWindow &renderWindow)
{
	renderWindowPointer = &renderWindow;
}
void MacOSWindow::createContext()
{
#ifdef USE_GL
#endif
}
void MacOSWindow::postInit()
{
}
bool MacOSWindow::pollMessages()
{
}
void MacOSWindow::swapBuffers()
{
}
void MacOSWindow::destroy()
{
	renderWindowPointer->iVendorRenderer->destroy();
}
void MacOSWindow::close()
{
}
void MacOSWindow::minimize()
{
}
void MacOSWindow::maximize()
{
}
void MacOSWindow::restore()
{
}
void MacOSWindow::warpPointer(glm::vec2 coords)
{
}
void MacOSWindow::setXY()
{
}
void MacOSWindow::setWidthHeight()
{
}
void MacOSWindow::mouseCapture(bool capture)
{
	if (capture)
	{}
	else
	{}
}
std::shared_ptr<IPlatformWindow> zg::modules::gl::createPlatformWindow()
{
	return std::shared_ptr<IPlatformWindow>(new MacOSWindow());
}
#endif