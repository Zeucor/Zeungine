#include <zg/Window.hpp>
#include <zg/vp/VML.hpp>
zg::components::scenes::SceneComponentCreateInfo ViewMouseLookFactory()
{
	zg::components::scenes::SceneComponentCreateInfo info{
		.name = "View Mouse Look",
		.onAttachedFunction = [](auto& component)
		{
			auto scenePointer = component.host;
			auto& scene = *scenePointer;
			auto& window = scene.window;
			component.make<zg::UniqueIdentifier>("mouseMoveID",
				window.addMouseMoveHandler(
					[scenePointer](glm::vec2 coords)
					{
						if (scenePointer->window.justWarpedPointer)
						{
							scenePointer->window.justWarpedPointer = false;
							return;
						}
						if (!scenePointer->window.focused)
							return;
						glm::vec2 center = {scenePointer->window.windowWidth / 2, scenePointer->window.windowHeight / 2};
						auto diff = coords - center;
						scenePointer->viewPointer->addPhiTheta(diff.x * 0.001f, -diff.y * 0.001f);
						scenePointer->window.warpPointer(center);
					}
				)
			);
			component.make<zg::UniqueIdentifier>("focusID",
				window.addFocusHandler(
					[scenePointer](bool focused)
					{
						if (focused)
							scenePointer->window.iPlatformWindow->hidePointer();
						else
							scenePointer->window.iPlatformWindow->showPointer();
					}
				)
			);
		},
		.onDetachedFunction = [](auto& component)
		{
			auto& window = component.host->window;
			window.removeMouseMoveHandler(component.getData<zg::UniqueIdentifier>("mouseMoveID"));
			window.removeFocusHandler(component.getData<zg::UniqueIdentifier>("focusID"));
		}
	};
	return info;
};