#pragma once
#ifdef _WIN32
#define ANEX_API __declspec(dllexport)
#else
#define ANEX_API __attribute__ ((visibility ("default")))
#endif
#include <hscpp/Hotswapper.h>
namespace anex::modules::gl
{
  struct GLWindow;
};
using namespace anex::modules::gl;
extern "C"
{
  ANEX_API void OnLoad(GLWindow &window);
  ANEX_API void OnHotswapLoad(GLWindow &window, hscpp::AllocationResolver &allocationResolver);
  ANEX_API void OnUnLoad(GLWindow &window);
}
