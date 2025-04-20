#include <zg/Window.hpp>
#include <zg/components/scenes/ViewMouseControl.hpp>
#include <zg/Registry.hpp>
zg::components::scenes::SceneComponentCreateInfo zg::components::scenes::ViewMouseControlFactory()
{
	zg::components::scenes::SceneComponentCreateInfo info{
		.name = "View Mouse Control",
		.onAttachedFunction = [](auto& component)
		{
			auto& scene = zg::Registry::getScene(component.hostIndexStack);
			auto& window = scene.window;
			component.make<zg::UniqueIdentifier>("mouseMoveID",
				window.addMouseMoveHandler(
					[hostIndexStack = component.hostIndexStack](glm::vec2 coords)
					{
						auto& scene = zg::Registry::getScene(hostIndexStack);
						if (scene.window.justWarpedPointer)
						{
							scene.window.justWarpedPointer = false;
							return;
						}
						if (!scene.window.focused)
							return;
						glm::vec2 center = {scene.window.windowWidth / 2, scene.window.windowHeight / 2};
						auto diff = coords - center;
						scene.viewPointer->addPhiTheta(diff.x * 0.001f, -diff.y * 0.001f);
						scene.window.warpPointer(center);
					}
				)
			);
			component.make<zg::UniqueIdentifier>("focusID",
				window.addFocusHandler(
					[hostIndexStack = component.hostIndexStack](bool focused)
					{
						auto& scene = zg::Registry::getScene(hostIndexStack);
						if (focused)
							scene.window.iPlatformWindow->hidePointer();
						else
							scene.window.iPlatformWindow->showPointer();
					}
				)
			);
		},
		.onDetachedFunction = [](auto& component)
		{
			auto& window = zg::Registry::getScene(component.hostIndexStack).window;
			window.removeMouseMoveHandler(component.getData<zg::UniqueIdentifier>("mouseMoveID"));
			window.removeFocusHandler(component.getData<zg::UniqueIdentifier>("focusID"));
		}
	};
	return info;
};