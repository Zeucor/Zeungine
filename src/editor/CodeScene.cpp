#include <editor/CodeScene.hpp>
using namespace anex::editor;
CodeScene::CodeScene(GLWindow &window):
	GLScene(window, {window.windowWidth / 2, window.windowHeight / 2, 50}, {0, 0, -1},
					{window.windowWidth, window.windowHeight})
{
	clearColor = {1, 1, 1, 1};
};