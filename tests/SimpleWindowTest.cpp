#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/Registry.hpp>
#include <iostream>
using namespace zg;
int main()
{
	auto sizeofWindow = sizeof(Window);
	auto sizeofScene = sizeof(Scene);
	auto sizeofEntity = sizeof(Entity);
	auto _25Entities1Window2Scene = (sizeofWindow + (sizeofScene * 2) + (sizeofEntity * 25));
	std::cout << "sizeof(Window)\t= " << sizeofWindow << "bytes" << std::endl <<
				"sizeof(Scene)\t= " << sizeofScene << "bytes" << std::endl <<
				"sizeof(Entity)\t= " << sizeofEntity << "bytes" << std::endl <<
				"sizeof(25E1W2S)\t= " << _25Entities1Window2Scene << "bytes" << std::endl;
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
