#include <editor/EditorScene.hpp>
using namespace zg::editor;
using namespace zg::modules::gl;
int32_t main()
{
	RenderWindow window("Editor", 1280, 720, -1, -1, true);
	window.runOnThread([&](auto& runningWindow)mutable
	{
		runningWindow.setIScene(std::make_shared<EditorScene>((RenderWindow&)runningWindow));
	});
	window.awaitWindowThread();
};