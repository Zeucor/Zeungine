#pragma once
#include <anex/modules/gl/GLWindow.hpp>
#ifdef _WIN32
#define ANEX_API __declspec(dllexport)
#else
#define ANEX_API __attribute__ ((visibility ("default")))
#endif
using namespace anex::modules::gl;
extern "C"
{
  ANEX_API void OnLoad(GLWindow &window);
  ANEX_API void OnUnLoad(GLWindow &window);
}
