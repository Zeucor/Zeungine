#include <zg/Window.hpp>
#include <zg/vp/VML.hpp>
#include <zg/Registry.hpp>
zg::components::scenes::SceneComponentCreateInfo zg::components::scenes::ViewMouseLookFactory()
{
	zg::components::scenes::SceneComponentCreateInfo info{
		.name = "View Mouse Look",
		.onAttachedFunction = [](auto& component)
		{
			auto& scene = zg::Registry::getScene(component.hostIDStack);
			auto& window = scene.window;
			component.make<zg::UniqueIdentifier>("mouseMoveID",
				window.addMouseMoveHandler(
					[hostIDStack = component.hostIDStack](glm::vec2 coords)
					{
						auto& scene = zg::Registry::getScene(hostIDStack);
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
					[hostIDStack = component.hostIDStack](bool focused)
					{
						auto& scene = zg::Registry::getScene(hostIDStack);
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
			auto& window = zg::Registry::getScene(component.hostIDStack).window;
			window.removeMouseMoveHandler(component.getData<zg::UniqueIdentifier>("mouseMoveID"));
			window.removeFocusHandler(component.getData<zg::UniqueIdentifier>("focusID"));
		}
	};
	return info;
};