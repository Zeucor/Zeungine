#pragma once
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
namespace zg::editor
{
	using namespace zg;
	struct CodeScene : Scene
	{
		explicit CodeScene(Window &window);
	};
}