#pragma once
#ifdef _WIN32
#define ZG_API __declspec(dllexport)
#else
#define ZG_API __attribute__ ((visibility ("default")))
#endif
#include <hscpp/Hotswapper.h>
namespace zg::modules::gl
{
  struct GLWindow;
};
using namespace zg::modules::gl;
extern "C"
{
  ZG_API void OnLoad(GLWindow &window);
  ZG_API void OnHotswapLoad(GLWindow &window, hscpp::AllocationResolver &allocationResolver);
  ZG_API void OnUnLoad(GLWindow &window);
}
