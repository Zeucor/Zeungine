#pragma once
#ifdef _WIN32
#include <windows.h>
#endif
#include <glad/gl.h>
#ifdef _WIN32
#include <GL/wglext.h>
#endif
namespace anex::modules::gl
{
	struct GLWindow;
	const bool GLcheck(GLWindow &window, const char* fn, const bool& egl = false);
}