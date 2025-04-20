#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/Registry.hpp>
using namespace zg;
int main()
{
	WindowCreateInfo windowCreateInfo{.title = "Simple Window", .windowWidth = 640, .windowHeight = 480};
	auto window_tuple = zg::Registry::addWindow(windowCreateInfo);
	auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);
	window.runOnThread(
		[](auto& window)
		{
			SceneCreateInfo sceneInfo{.name = "Simple Scene",
																.onAttachedFunction = [](auto& scene) { scene.clearColor = {1, 0, 0, 1}; }};
			window.addScene(sceneInfo);
		});
	window.run();
}
