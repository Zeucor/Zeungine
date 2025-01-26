#pragma once
#include <zg/modules/gl/GLScene.hpp>
#include <zg/modules/gl/RenderWindow.hpp>
namespace zg::editor
{
	using namespace zg::modules::gl;
	struct CodeScene : GLScene
	{
		explicit CodeScene(RenderWindow &window);
	};
}