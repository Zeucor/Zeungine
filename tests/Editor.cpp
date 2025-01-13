#include <anex/modules/gl/GLWindow.hpp>
#include <anex/SharedLibrary.hpp>
#include <anex/RuntimeAPI.hpp>
using namespace anex;
using namespace anex::modules::gl;

int main()
{
	GLWindow window("Editor", 1280, 720, -1, -1);
	window.clearColor = {0, 0, 0, 1};
	SharedLibrary gameLibrary("EditorGame.dll");
	auto load = gameLibrary.getProc<LOAD_FUNCTION>("Load");
	GLWindow *childWindowPointer = 0;
	window.runOnThread([&](auto &window)
	{
		auto childWindowWidth = 640;
		auto childWindowHeight = 480;
	  auto &childWindow = window.createChildWindow(
			"EditorChild",
			childWindowWidth,
			childWindowHeight,
window.windowWidth / 2 - childWindowWidth / 2,
			0);
		childWindowPointer = (GLWindow *)&childWindow;
		childWindow.runOnThread([&](auto &window)
		{
			load((GLWindow &)window);
		});
	});
	while (!childWindowPointer) {}
	childWindowPointer->awaitWindowThread();
	window.awaitWindowThread();
};