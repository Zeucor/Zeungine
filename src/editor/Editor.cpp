#include <editor/EditorScene.hpp>
using namespace zg::editor;
using namespace zg;
int32_t main()
{
	Window window("Editor", 1280, 720, -1, -1, true);
	window.runOnThread([&](auto &runningWindow) mutable
					   { runningWindow.setScene(std::make_shared<EditorScene>((Window &)runningWindow)); });
	window.run();
};