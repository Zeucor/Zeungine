#include <zg/Window.hpp>
#include <zg/Scene.hpp>
#include <zg/vp/VML.hpp>
#include <zg/entities/Cube.hpp>
using namespace zg;
struct ExampleScene : Scene
{
	vp::VML vml; // view mouse look
	ExampleScene(Window &window):
			Scene(window,
						{0, 10, 10}, // camera position
						glm::normalize(glm::vec3(0, -1, -1)), //camera direction
						81.f // fov
			),
			vml(*this)
	{
		addEntity(std::make_shared<entities::Cube>(
				(Window &)window, // reference to window
				*this, // reference to scene
				glm::vec3(0, 0, 0), // position
				glm::vec3(0, 0, 0), // rotation
				glm::vec3(1, 1, 1), // scale
				glm::vec3(2, 1, 4), // cube size
				shaders::RuntimeConstants() // additional shader constants
		));
	};
};
int main()
{
	Window window("My Window Title", 640, 480, -1, -1);
	window.runOnThread([](auto &window)
	{
			window.setIScene(std::make_shared<ExampleScene>((Window &)window));
	});
	window.run();
}