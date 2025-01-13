#pragma once
#ifdef RUNTIME_EXPORTS
#define RUNTIME_EXPORT __declspec(dllexport)
#else
#define RUNTIME_EXPORT __declspec(dllimport)
#endif
#include <anex/modules/gl/GLWindow.hpp>
using namespace anex::modules::gl;
extern "C"
{
	RUNTIME_EXPORT void Load(GLWindow &window);
}
#define LOAD_FUNCTION void(*)(GLWindow &)