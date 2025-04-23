#include <zg/Window.hpp>
#include <zg/components/scenes/ViewMouseControl.hpp>
#include <zg/Registry.hpp>
#include <zg/physics/AABB.hpp>
#include <chrono>
zg::components::scenes::SceneComponentCreateInfo zg::components::scenes::ViewMouseControlFactory()
{
	zg::components::scenes::SceneComponentCreateInfo info{
		.name = "View Mouse Control",
		.onAttachedFunction = [](auto& component)
		{
			auto& scene = zg::Registry::getScene(component.hostIndexStack);
			auto& window = scene.window;
			auto iRenderer = window.iRenderer;
            auto& deadZonePercent = component.template make<float>("DeadZonePercent", 0.1f);
            auto& lastPosition = component.template make<glm::vec2>("LastPosition", 0.0f, 0.0f);
			glm::vec2 center = {scene.window.windowWidth / 2.0f, scene.window.windowHeight / 2.0f};
			float boxHalfWidth = scene.window.windowWidth * (deadZonePercent * 0.5f);
			float boxHalfHeight = scene.window.windowHeight * (deadZonePercent * 0.5f);
			zg::physics::AABB<2> centerBox(glm::vec2(center.x - boxHalfWidth, center.y - boxHalfHeight),
																		glm::vec2(center.x + boxHalfWidth, center.y + boxHalfHeight));
			int* count = new int();
			component.template make<zg::UniqueIdentifier>("mouseMoveID",
				window.addMouseMoveHandler(
					[&, centerBox, center, count](glm::vec2 coords)mutable
					{
						auto diff = coords - lastPosition;
						if (!diff.x && !diff.y)
							return;
						scene.viewPointer->addPhiTheta(diff.x * 0.001f, -diff.y * 0.001f);
						if (!centerBox.isPointInside(coords))
						{
							scene.window.warpPointer(center);
							lastPosition = center;
						}
						else
						{
							lastPosition = coords;
						}
					}
				)
			);
			component.template make<zg::UniqueIdentifier>("focusID",
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
			window.removeMouseMoveHandler(component.template getData<zg::UniqueIdentifier>("mouseMoveID"));
			window.removeFocusHandler(component.template getData<zg::UniqueIdentifier>("focusID"));
		}
	};
	return info;
};