#pragma once
#ifdef _WIN32
#define ZG_API __declspec(dllexport)
#else
#define ZG_API __attribute__ ((visibility ("default")))
#endif
#include <hscpp/Hotswapper.h>
namespace zg::modules::gl
{
  struct RenderWindow;
};
using namespace zg::modules::gl;
extern "C"
{
  ZG_API void OnLoad(RenderWindow &window);
  ZG_API void OnHotswapLoad(RenderWindow &window, hscpp::AllocationResolver &allocationResolver);
  ZG_API void OnUnLoad(RenderWindow &window);
}
