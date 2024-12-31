#pragma once
#ifdef _WIN32
#include <windows.h>
#include <glad/glad.h>
#include <GL/glext.h>
#include <GL/wglext.h>
#endif
namespace anex::modules::gl
{
	const bool GLcheck(const char* fn, const bool& egl = false);
}