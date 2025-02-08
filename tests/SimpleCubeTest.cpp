#include <zg/Window.hpp>
#include <zg/Scene.hpp>
#include <zg/vp/VML.hpp>
#include <zg/entities/Cube.hpp>
#include <iostream>
using namespace zg;
struct ExampleScene : Scene
{
	vp::VML vml; // view mouse look
	long double deltaTimeCounter = 0;
	uint32_t mPressID = 0;
	uint32_t nPressID = 0;
	uint32_t rPressID = 0;
	uint32_t qPressID = 0;
	ExampleScene(Window &_window) : Scene(_window,
										  {0, 10, 10},							// camera position
										  glm::normalize(glm::vec3(0, -1, -1)), // camera direction
										  81.f									// fov
										  ),
									vml(*this)
	{
		clearColor = {1, 0, 1, 1};
		addEntity(std::make_shared<entities::Cube>(
			window,						// reference to window
			*this,						// reference to scene
			glm::vec3(0, 0, 0),			// position
			glm::vec3(0, 0, 0),			// rotation
			glm::vec3(1, 1, 1),			// scale
			glm::vec3(2, 1, 4),			// cube size
			shaders::RuntimeConstants() // additional shader constants
			));
		mPressID = window.addKeyPressHandler('m', [&](auto pressed)
											 {
			if (pressed)
				window.maximize(); });
		nPressID = window.addKeyPressHandler('n', [&](auto pressed)
											 {
			if (pressed)
				window.minimize(); });
		rPressID = window.addKeyPressHandler('r', [&](auto pressed)
											 {
			if (pressed)
				window.restore(); });
		qPressID = window.addKeyPressHandler('q', [&](auto pressed)
											 {
			if (pressed)
				window.close(); });
		window.setXY(320, 320);
	}
	~ExampleScene()
	{
		window.removeKeyPressHandler('m', mPressID);
		window.removeKeyPressHandler('n', nPressID);
		window.removeKeyPressHandler('r', rPressID);
	}
	void update() override
	{
		deltaTimeCounter += window.deltaTime;
		clearColor = {std::sin(deltaTimeCounter), std::cos(deltaTimeCounter), std::tan(deltaTimeCounter), 1};
	}
};
int main()
{
	Window window("Cube Test", 1024, 768, -1, -1, true, false);
	window.runOnThread([](auto &window)
					   { window.setIScene(std::make_shared<ExampleScene>(window)); });
	window.run();
}