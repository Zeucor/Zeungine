#pragma once
#ifdef RUNTIME_EXPORTS
#define RUNTIME_EXPORT __declspec(dllexport)
#else
#define RUNTIME_EXPORT __declspec(dllimport)
#endif
#include <zg/Window.hpp>
using namespace zg;
extern "C"
{
	RUNTIME_EXPORT void Load(Window &window);
}
#define LOAD_FUNCTION void(*)(Window &)