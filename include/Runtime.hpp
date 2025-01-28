#pragma once
#ifdef _WIN32
#define ZG_API extern "C" __declspec(dllexport)
#else
#define ZG_API extern "C" __attribute__ ((visibility ("default")))
#endif
#include <hscpp/Hotswapper.h>
namespace zg
{
  struct Window;
};
using namespace zg;
ZG_API void OnLoad(Window &window);
ZG_API void OnHotswapLoad(Window &window, hscpp::AllocationResolver &allocationResolver);
ZG_API void OnUnLoad(Window &window);
