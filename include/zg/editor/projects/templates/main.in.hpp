#pragma once
#include <zg/Runtime.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
using namespace zg;
using namespace zg;
struct MainScene : Scene
{
	explicit MainScene(Window &window);
};
ZG_API void OnLoad(Window& window);