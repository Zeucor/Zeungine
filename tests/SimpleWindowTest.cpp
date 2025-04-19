#include <zg/Scene.hpp>
#include <zg/Window.hpp>
using namespace zg;
int main()
{
	WindowCreateInfo windowInfo{.title = "Simple Window", .windowWidth = 640, .windowHeight = 480};
	Window window(windowInfo);
	window.runOnThread(
		[](auto& window)
		{
			SceneCreateInfo sceneInfo{.name = "Simple Scene",
																.onAttachedFunction = [](auto& scene) { scene.clearColor = {1, 0, 0, 1}; }};
			window.addScene(sceneInfo);
		});
	window.run();
}
