#include <zg/editor/CodeScene.hpp>
zg::SceneCreateInfo CodeSceneFactory()
{
	zg::SceneCreateInfo info{
		.name = "Code Scene",
		.onAttachedFunction = [](auto& scene)
		{
			scene.clearColor = {1, 1, 1, 1};
		}
	};
	return info;
}