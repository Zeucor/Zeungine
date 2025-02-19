#include <main.hpp>
#include <zg/entities/Cube.hpp>
#include <iostream>
MainScene::MainScene(Window& _window):
	Scene(_window, {0, 10, 10}, {0, -1, -1}, 80.f),
	cube(make_shared<entities::Cube>(
		window, // reference to window
		*this, // reference to scene
		glm::vec3(0, 0, 0), // position
		glm::vec3(0, 0, 0), // rotation
		glm::vec3(1, 1, 1), // scale
		glm::vec3(5, 5, 5) // cube size
	))
{
	clearColor = {0.5, 0, 1.0, 1};
	addEntity(cube);
	upKeyID = window.addKeyUpdateHandler(KEYCODE_UP, [&]
	{
		cube->position.y += 7 * window.deltaTime;
	});
	downKeyID = window.addKeyUpdateHandler(KEYCODE_DOWN, [&]
	{
		cube->position.y -= 7 * window.deltaTime;
	});
	leftKeyID = window.addKeyUpdateHandler(KEYCODE_LEFT, [&]
	{
		cube->position.x -= 7 * window.deltaTime;
	});
	rightKeyID = window.addKeyUpdateHandler(KEYCODE_RIGHT, [&]
	{
		cube->position.x += 7 * window.deltaTime;
	});
}
MainScene::~MainScene()
{
	window.removeKeyUpdateHandler(KEYCODE_UP, upKeyID);
	window.removeKeyUpdateHandler(KEYCODE_DOWN, downKeyID);
	window.removeKeyUpdateHandler(KEYCODE_LEFT, leftKeyID);
	window.removeKeyUpdateHandler(KEYCODE_RIGHT, rightKeyID);
}
ZG_API void OnLoad(Window& window)
{
	window.setScene(make_shared<MainScene>(window));
}