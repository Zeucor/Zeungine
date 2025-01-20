#pragma once
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/GLWindow.hpp>
namespace anex::editor
{
	using namespace anex::modules::gl;
	struct CodeScene : GLScene
	{
		explicit CodeScene(GLWindow &window);
	};
}