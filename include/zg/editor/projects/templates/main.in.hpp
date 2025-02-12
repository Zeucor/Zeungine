#pragma once
#include <zg/Runtime.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Cube.hpp>
using namespace zg;
struct MainScene : Scene
{
	std::shared_ptr<entities::Cube> cube;
	EventIdentifier leftKeyID;
	EventIdentifier rightKeyID;
	EventIdentifier upKeyID;
	EventIdentifier downKeyID;
	explicit MainScene(Window &window);
	~MainScene();
};
ZG_API void OnLoad(Window& window);