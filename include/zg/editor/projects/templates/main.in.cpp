#include <main.hpp>
#include <zg/entities/Cube.hpp>
MainScene::MainScene(Window& window):
	Scene(window, {0, 50, 50}, {0, -1, -1}, 80.f)
{
	clearColor = {0.5, 0, 0.5, 1};
	addEntity(std::make_shared<entities::Cube>(
		window, // reference to window
		*this, // reference to scene
		glm::vec3(0, 0, 0), // position
		glm::vec3(0, 0, 0), // rotation
		glm::vec3(1, 1, 1), // scale
		glm::vec3(5, 5, 5), // cube size
		shaders::RuntimeConstants() // additional shader constants
	));
}
ZG_API void OnLoad(Window& window)
{
	window.setScene(std::make_shared<MainScene>(window));
}