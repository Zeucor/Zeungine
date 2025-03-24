#pragma once
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
namespace zg::editor
{
	struct CodeScene : zg::Scene
	{
		explicit CodeScene(zg::Window &window);
	};
}