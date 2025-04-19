#pragma once
#include <zg/Runtime.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Cube.hpp>
struct MainScene : zg::Scene
{
	Entity* cube;
	UniqueIdentifier leftKeyID;
	UniqueIdentifier rightKeyID;
	UniqueIdentifier upKeyID;
	UniqueIdentifier downKeyID;
	explicit MainScene(zg::Window &window);
	~MainScene();
};
ZG_API void OnLoad(zg::Window& window);