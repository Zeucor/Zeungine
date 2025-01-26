#include <editor/CodeScene.hpp>
using namespace zg::editor;
CodeScene::CodeScene(RenderWindow &window):
	GLScene(window, {window.windowWidth / 2, window.windowHeight / 2, 50}, {0, 0, -1},
					{window.windowWidth, window.windowHeight})
{
	clearColor = {1, 1, 1, 1};
};