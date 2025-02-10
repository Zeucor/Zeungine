#pragma once
#ifdef _WIN32
#define ZG_API extern "C" __declspec(dllexport)
#else
#define ZG_API extern "C" __attribute__((visibility("default")))
#endif
namespace zg
{
  struct Window;
};
using namespace zg;
ZG_API void OnLoad(Window &window);