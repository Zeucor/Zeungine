#include <zg/editor/EditorScene.hpp>
#include <zg/Window.hpp>
#include <zg/system/TerminalIO.hpp>
using namespace zg::system;
using namespace zg;
int32_t main()
{
	TeIO teio(true);
	teio.echo(false);
	teio.canonical(false);
	teio.setProfile();
	WindowCreateInfo windowCreateInfo{
		.title = "Editor",
		.windowWidth = 1280,
		.windowHeight = 720,
		.windowX = -1,
		.windowY = -1,
		.borderless = true,
		.vsync = false
	};
	Window window(windowCreateInfo);
	window.addScene(EditorSceneFactory());
	window.run();
	return 0;
};