#include <zg/Window.hpp>
#include <zg/components/scenes/ViewMouseControl.hpp>
#include <zg/Registry.hpp>
#include <zg/physics/AABB.hpp>
#include <chrono>
using namespace zg;
components::scenes::SceneComponentCreateInfo components::scenes::ViewMouseControlFactory()
{
	components::scenes::SceneComponentCreateInfo info{
		.name = "View Mouse Control",
		.onAttachedFunction = [](auto& component)
		{
			auto& scene = Registry::getScene(component.HOST_INDEX_STACK);
			auto& window = Registry::getWindow(component.HOST_INDEX_STACK);
			auto iRenderer = window.iRenderer;
            auto& deadZonePercent = component.template make<float>("DeadZonePercent", 0.1f);
            auto& lastPosition = component.template make<glm::vec2>("LastPosition", 0.0f, 0.0f);
			glm::vec2 center = {window.windowWidth / 2.0f, window.windowHeight / 2.0f};
			float boxHalfWidth = window.windowWidth * (deadZonePercent * 0.5f);
			float boxHalfHeight = window.windowHeight * (deadZonePercent * 0.5f);
			physics::AABB<2> centerBox(glm::vec2(center.x - boxHalfWidth, center.y - boxHalfHeight),
																		glm::vec2(center.x + boxHalfWidth, center.y + boxHalfHeight));
			int* count = new int();
			component.template make<UniqueIdentifier>("mouseMoveID",
				window.addMouseMoveHandler(
					[&, centerBox, center, count](glm::vec2 coords)mutable
					{
						auto diff = coords - lastPosition;
						if (!diff.x && !diff.y)
							return;
						scene.viewPointer->addPhiTheta(diff.x * 0.001f, -diff.y * 0.001f);
						if (!centerBox.isPointInside(coords))
						{
							window.warpPointer(center);
							lastPosition = center;
						}
						else
						{
							lastPosition = coords;
						}
					}
				)
			);
			component.template make<UniqueIdentifier>("focusID",
				window.addFocusHandler(
					[HOST_INDEX_STACK = component.HOST_INDEX_STACK](bool focused)
					{
						auto& window = Registry::getWindow(HOST_INDEX_STACK);
						if (focused)
							window.iPlatformWindow->hidePointer();
						else
							window.iPlatformWindow->showPointer();
					}
				)
			);
		},
		.onDetachedFunction = [](auto& component)
		{
			auto& window = Registry::getWindow(component.HOST_INDEX_STACK);
			window.removeMouseMoveHandler(component.template getData<UniqueIdentifier>("mouseMoveID"));
			window.removeFocusHandler(component.template getData<UniqueIdentifier>("focusID"));
		}
	};
	return info;
};