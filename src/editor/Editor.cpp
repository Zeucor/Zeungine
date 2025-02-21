#include <zg/editor/EditorScene.hpp>
#include <zg/system/TerminalIO.hpp>
using namespace zg::editor;
using namespace zg::system;
using namespace zg;
int32_t main()
{
	TeIO teio(true);
	teio.echo(false);
	teio.canonical(false);
	teio.setProfile();
	Window window("Editor", 1280, 720, -1, -1, true, false);
	window.runOnThread([&](auto &runningWindow) mutable
					   { runningWindow.setScene(std::make_shared<EditorScene>((Window &)runningWindow)); });
	window.run();
	return 0;
};