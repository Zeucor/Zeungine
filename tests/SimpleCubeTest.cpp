#include <iostream>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/components/scenes/ViewMouseControl.hpp>
#include <zg/Registry.hpp>
#include <zg/components/scenes/ViewQuadKeyControl.hpp>
using namespace zg;
auto cubeCreateInfo = entities::CubeFactory("Basic Red Cube", {0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1}, {2, 1, 3});
auto planeAngle = glm::angleAxis(glm::radians(-90.f), glm::vec3(1, 0, 0));
auto planeCreateInfo =
	entities::PlaneFactory({0.3, 0.25, 0.35, 0.75}, "Basic Grey Plane", {0, -0.5, 0}, planeAngle, {1, 1, 1}, {100, 100});
SceneCreateInfo ExampleSceneFactory();
int main()
{
	WindowCreateInfo windowCreateInfo{.title = "Cube Test", .borderless = true, .vsync = false, .framerate = 144};
	auto window_tuple = zg::Registry::addWindow(windowCreateInfo);
	auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);
	window.runOnThread(
		[](auto& window)
		{
			auto sceneCreateInfo = ExampleSceneFactory();
			window.addScene(sceneCreateInfo);
		});
	window.run();
}
SceneCreateInfo ExampleSceneFactory()
{
	SceneCreateInfo info{
		.name = "ExampleScene",
		.cameraPosition = glm::vec3(0, 10, 10),
		.cameraDirection = glm::normalize(glm::vec3(0, -1, -1)),
		.onAttachedFunction =
			[](auto& scene)
		{
			scene.clearColor = {1, 0, 1, 1};
			scene.template setData<size_t>("CubeID", std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(cubeCreateInfo)));
			scene.template setData<size_t>("PlaneID", std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(planeCreateInfo)));
			scene.template setData<zg::UniqueIdentifier>("mPressID",
																					scene.window.addKeyPressHandler('m',
																																					[&](auto pressed)
																																					{
																																						if (pressed)
																																							scene.window.maximize();
																																					}));
			scene.template setData<zg::UniqueIdentifier>("nPressID",
																					scene.window.addKeyPressHandler('n',
																																					[&](auto pressed)
																																					{
																																						if (pressed)
																																							scene.window.minimize();
																																					}));
			scene.template setData<zg::UniqueIdentifier>("rPressID",
																					scene.window.addKeyPressHandler('r',
																																					[&](auto pressed)
																																					{
																																						if (pressed)
																																							scene.window.restore();
																																					}));
			scene.template setData<zg::UniqueIdentifier>("qPressID",
																					scene.window.addKeyPressHandler('q',
																																					[&](auto pressed)
																																					{
																																						if (pressed)
																																							scene.window.close();
																																					}));

			scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
			scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 3));
			scene.template make<float>("deltaTimeCounter", 0.f);
		},
		.onDetachedFunction =
			[](auto& scene)
		{
			scene.window.removeKeyPressHandler('m', scene.template getData<zg::UniqueIdentifier>("mPressID"));
			scene.window.removeKeyPressHandler('n', scene.template getData<zg::UniqueIdentifier>("nPressID"));
			scene.window.removeKeyPressHandler('r', scene.template getData<zg::UniqueIdentifier>("rPressID"));
			scene.window.removeKeyPressHandler('q', scene.template getData<zg::UniqueIdentifier>("qPressID"));
		},
		.preUpdateFunction =
			[](auto& scene)
		{
			auto deltaTimeCounter = (scene.template getData<float>("deltaTimeCounter") += scene.window.deltaTime);
			scene.clearColor = {std::sin(deltaTimeCounter), std::cos(deltaTimeCounter), std::tan(deltaTimeCounter), 1};
		}};
	return info;
};
