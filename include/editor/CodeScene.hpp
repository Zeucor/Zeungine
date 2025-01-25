#pragma once
#include <zg/modules/gl/GLScene.hpp>
#include <zg/modules/gl/GLWindow.hpp>
namespace zg::editor
{
	using namespace zg::modules::gl;
	struct CodeScene : GLScene
	{
		explicit CodeScene(GLWindow &window);
	};
}