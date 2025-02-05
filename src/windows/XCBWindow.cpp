#ifdef LINUX
#ifdef USE_XCB
#include <zg/windows/XCBWindow.hpp>
#include <iostream>
#include <zg/common.hpp>
#include <iostream>
using namespace zg;
void XCBWindow::init(Window& renderWindow)
{
}
void XCBWindow::postInit()
{
}
bool XCBWindow::pollMessages()
{
	return true;
}
void XCBWindow::swapBuffers()
{
}
void XCBWindow::destroy()
{
}
void XCBWindow::close()
{
}
void XCBWindow::minimize()
{
}
void XCBWindow::maximize()
{
}
void XCBWindow::restore()
{
}
void XCBWindow::warpPointer(glm::vec2 coords)
{
}
void XCBWindow::showPointer()
{
}
void XCBWindow::hidePointer()
{
}
void XCBWindow::setXY()
{
}
void XCBWindow::setWidthHeight()
{
}
void XCBWindow::mouseCapture(bool capture)
{
}
#endif
#endif