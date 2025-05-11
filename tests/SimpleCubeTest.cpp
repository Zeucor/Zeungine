#include <iostream>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/components/scenes/ViewMouseControl.hpp>
#include <zg/Registry.hpp>
#include <zg/components/scenes/ViewQuadKeyControl.hpp>
#include <zg/crypto/Random.hpp>
#include <zg/components/scenes/Bloom.hpp>
using namespace zg;
auto cubeCreateInfo = entities::CubeFactory("Basic Red Cube", {0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1});
// auto planeAngle = glm::angleAxis(glm::radians(-90.f), glm::vec3(0, 0, 0));
auto planeCreateInfo = entities::PlaneFactory({0.3, 0.25, 0.35, 0.75}, "Basic Grey Plane", {0, -0.5, 0}, {1, 0, 0, 0}, {100, 100, 100});
SceneCreateInfo ExampleSceneFactory();
int main()
{
	WindowCreateInfo windowCreateInfo{.title = "Cube Test", .borderless = true, .vsync = false, .framerate = 144};
	auto window_tuple = zg::Registry::addWindow(windowCreateInfo);
	auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);
	window.runOnThread([](auto& window) {
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
		.onAttachedFunction = [](auto& scene)
		{
			auto& window = Registry::getWindow(scene.INDEX_STACK);
			scene.clearColor = {1, 0, 1, 1};
			auto& cubes = scene.template make<std::vector<size_t>>("CubeIDs");
			cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(cubeCreateInfo)));
			for (float x = 1; x < 20; x += 4)
			{
				for (float z = 1; z < 20; z += 4)
				{
					auto usingCubeCreateInfo = cubeCreateInfo;
					usingCubeCreateInfo.position.x += x;
					usingCubeCreateInfo.position.z += z;
					usingCubeCreateInfo.meshInfos[0].material.albedo = glm::vec4(0.95, 0.95, 0.95, 1);
					cubes.push_back(std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(usingCubeCreateInfo)));
				}
			}
			scene.template setData<size_t>("PlaneID", std::get<KEY_ID_VECTOR_ID_INDEX>(scene.addEntity(planeCreateInfo)));
			// scene.template setData<zg::UniqueIdentifier>(
			// 	"mPressID",
			// 	window.addKeyPressHandler('m',
			// 		[&](auto pressed)
			// 		{
			// 			if (pressed)
			// 				window.maximize();
			// 		}
			// 	)
			// );
			// scene.template setData<zg::UniqueIdentifier>(
			// 	"nPressID",
			// 	window.addKeyPressHandler('n',
			// 		[&](auto pressed)
			// 		{
			// 			if (pressed)
			// 				window.minimize();
			// 		}
			// 	)
			// );
			// scene.template setData<zg::UniqueIdentifier>(
			// 	"rPressID",
			// 	window.addKeyPressHandler('r',
			// 		[&](auto pressed)
			// 		{
			// 			if (pressed)
			// 				window.restore();
			// 		}
			// 	)
			// );
			scene.template setData<zg::UniqueIdentifier>(
				"qPressID",
				window.addKeyPressHandler('q',
					[INDEX_STACK = scene.INDEX_STACK](auto pressed)
					{
						if (pressed)
						{
							auto& window = Registry::getWindow(INDEX_STACK);
							window.close();
						}
					}
				)
			);
			scene.attachComponent(zg::components::scenes::ViewMouseControlFactory());
			scene.attachComponent(zg::components::scenes::ViewQuadKeyControlFactory(zg::components::scenes::KeyScheme::WSADSC, 3));
			scene.template make<float>("deltaTimeCounter", 0.f);
			scene.attachComponent(zg::components::scenes::BloomFactory());
		},
		.onDetachedFunction = [](auto& scene)
		{
			auto& window = Registry::getWindow(scene.INDEX_STACK);
			window.removeKeyPressHandler('m', scene.template getData<zg::UniqueIdentifier>("mPressID"));
			window.removeKeyPressHandler('n', scene.template getData<zg::UniqueIdentifier>("nPressID"));
			window.removeKeyPressHandler('r', scene.template getData<zg::UniqueIdentifier>("rPressID"));
			window.removeKeyPressHandler('q', scene.template getData<zg::UniqueIdentifier>("qPressID"));
		},
		.preUpdateFunction = [](auto& scene)
		{
			auto& window = Registry::getWindow(scene.INDEX_STACK);
			auto deltaTimeCounter = (scene.template getData<float>("deltaTimeCounter") += window.deltaTime);
			scene.clearColor = {std::sin(deltaTimeCounter), std::cos(deltaTimeCounter), std::tan(deltaTimeCounter), 1};

			auto& cubes = scene.template getData<std::vector<size_t>>("CubeIDs");
			auto cubesSize = cubes.size();
			auto cubesData = cubes.data();
			for (size_t i = 0; i < cubesSize; ++i)
			{
				auto& cubeID = cubesData[i];
				auto& cube = Registry::getEntity(cubeID);
				auto iNorm = (i / (float)cubesSize);
				auto iNorm_1m = 1.0 - iNorm;
				cube.position.y = sin((scene.updateNonce + (iNorm * (acos(-1.0) * 2.0)))) + 
				cos((scene.updateNonce + (iNorm_1m * (acos(-1.0) * 2.0)))) + 1 + 
				tan((scene.updateNonce + (iNorm_1m * iNorm_1m * (acos(-1.0) * 4.0))));
			}
		}};
	return info;
};
