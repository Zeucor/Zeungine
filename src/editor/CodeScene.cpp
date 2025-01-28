#include <editor/CodeScene.hpp>
using namespace zg::editor;
CodeScene::CodeScene(Window &window):
	Scene(window, {window.windowWidth / 2, window.windowHeight / 2, 50}, {0, 0, -1},
					{window.windowWidth, window.windowHeight})
{
	clearColor = {1, 1, 1, 1};
};