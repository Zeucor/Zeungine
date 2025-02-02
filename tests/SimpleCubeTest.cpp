#include <zg/Window.hpp>
#include <zg/Scene.hpp>
#include <zg/vp/VML.hpp>
#include <zg/entities/Cube.hpp>
using namespace zg;
struct ExampleScene : Scene
{
	vp::VML vml; // view mouse look
	float deltaTimeCounter = 0;
	ExampleScene(Window &window):
			Scene(window,
						{0, 10, 10}, // camera position
						glm::normalize(glm::vec3(0, -1, -1)), //camera direction
						81.f // fov
			),
			vml(*this)
	{
		clearColor = {1, 0, 1, 1};
		addEntity(std::make_shared<entities::Cube>(
				window, // reference to window
				*this, // reference to scene
				glm::vec3(0, 0, 0), // position
				glm::vec3(0, 0, 0), // rotation
				glm::vec3(1, 1, 1), // scale
				glm::vec3(2, 1, 4), // cube size
				shaders::RuntimeConstants() // additional shader constants
		));
	};
	void update() override
	{
		deltaTimeCounter += window.deltaTime;
		clearColor = {std::sin(deltaTimeCounter), std::cos(deltaTimeCounter), std::tan(deltaTimeCounter), 1};
	}
};
int main()
{
	Window window("Cube Test", 640, 480, -1, -1);
	window.runOnThread([](auto &window)
	{
		window.setIScene(std::make_shared<ExampleScene>(window));
	});
	window.run();
}