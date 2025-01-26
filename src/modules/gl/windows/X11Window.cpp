#ifdef LINUX
#include <zg/modules/gl/windows/X11Window.hpp>
#include <zg/modules/gl/entities/Plane.hpp>
#include <iostream>
using namespace zg::modules::gl;
void X11Window::init(RenderWindow &renderWindow)
{
	renderWindowPointer = &renderWindow;
}
void X11Window::createContext()
{
#ifdef USE_GL
#endif
}
void X11Window::postInit()
{
}
bool X11Window::pollMessages()
{
}
void X11Window::swapBuffers()
{
}
void X11Window::destroy()
{
	renderWindowPointer->iVendorRenderer->destroy();
}
void X11Window::close()
{
}
void X11Window::minimize()
{
}
void X11Window::maximize()
{
}
void X11Window::restore()
{
}
void X11Window::warpPointer(glm::vec2 coords)
{
}
void X11Window::setXY()
{
}
void X11Window::setWidthHeight()
{
}
void X11Window::mouseCapture(bool capture)
{
	if (capture)
	{}
	else
	{}
}
std::shared_ptr<IPlatformWindow> zg::modules::gl::createPlatformWindow()
{
	return std::shared_ptr<IPlatformWindow>(new X11Window());
}
#endif