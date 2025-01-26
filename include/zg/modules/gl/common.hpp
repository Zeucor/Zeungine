#pragma once
#ifdef _WIN32
#include <windows.h>
#endif
#include <glad/gl.h>
#ifdef _WIN32
#include <GL/wglext.h>
#endif
namespace zg::modules::gl
{
	struct RenderWindow;
	const bool GLcheck(RenderWindow &window, const char* fn, const bool egl = false);
}