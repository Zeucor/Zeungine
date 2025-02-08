#include <zg/Window.hpp>
#include <zg/Scene.hpp>
using namespace zg;
struct SimpleScene : Scene
{
	SimpleScene(Window &window) : Scene(window, {0, 10, 10}, {0, -1, -1}, 81.f)
	{
		clearColor = {1, 0, 0, 1};
	};
};
int main()
{
	Window window("My Simple Window", 640, 480, -1, -1);
	window.runOnThread([](auto &window)
					   { window.setIScene(std::make_shared<SimpleScene>(window)); });
	window.run();
}