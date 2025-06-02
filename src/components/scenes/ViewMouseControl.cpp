#include <zg/Window.hpp>
#include <zg/components/scenes/ViewMouseControl.hpp>
#include <zg/Registry.hpp>
#include <zg/physics/AABB.hpp>
#include <zg/interfaces/IPlatformWindow.hpp>
#include <chrono>
using namespace zg;
components::scenes::SceneComponentCreateInfo components::scenes::ViewMouseControlFactory()
{
	components::scenes::SceneComponentCreateInfo info{
		.name = "View Mouse Control",
		.onAttachedFunction = [](auto& component)
		{
			auto& scene = Registry::GetSingleton().getScene(component.HOST_INDEX_STACK);
			auto& window = Registry::GetSingleton().getWindow(component.HOST_INDEX_STACK);
			auto iRenderer = window.iRenderer;
            auto& deadZonePercent = component.template make<float>("DeadZonePercent", 0.1f);
            auto& lastPosition = component.template make<glm::vec2>("LastPosition", 0.0f, 0.0f);
			glm::vec2 center = {(const float&)window.windowWidth / 2.0f, (const float&)window.windowHeight / 2.0f};
			float boxHalfWidth = (const float&)window.windowWidth * (deadZonePercent * 0.5f);
			float boxHalfHeight = (const float&)window.windowHeight * (deadZonePercent * 0.5f);
			physics::AABB<2> centerBox(glm::vec2(center.x - boxHalfWidth, center.y - boxHalfHeight),
																		glm::vec2(center.x + boxHalfWidth, center.y + boxHalfHeight));
			int* count = new int();
			component.template make<bool>("AltPressed", false);
			window.iPlatformWindow->hidePointer();
			component.template make<size_t>("AltPressID", window.registerHandler(EVENT_KEY_PRESS, [
				SCENE_INDEX_STACK = scene.INDEX_STACK,
				component_ID = component.ID
			](auto& event) {
				if (event.getValue() != KEYCODE_ALT)
					return;
				auto& pressed = event.template castData<bool>();
				auto& scene = Registry::GetSingleton().getScene(SCENE_INDEX_STACK);
				auto& component = scene.getComponentByID(component_ID);
				auto& altPressed = component.template getData<bool>("AltPressed");
				altPressed = pressed;
				auto& window = Registry::GetSingleton().getWindow(SCENE_INDEX_STACK);
				if (altPressed)
				{
					window.iPlatformWindow->showPointer();
				}
				else
				{
					window.iPlatformWindow->hidePointer();
				}
			}));
			component.template make<UniqueIdentifier>("MouseMoveID",
				window.registerHandler(EVENT_MOUSE_MOVE,
					[centerBox, center, count, componentID = component.ID, SCENE_INDEX_STACK = scene.INDEX_STACK](auto& event)mutable
					{
						auto& coords = event.template castData<glm::vec2>();
						auto& scene = Registry::GetSingleton().getScene(SCENE_INDEX_STACK);
						auto& window = Registry::GetSingleton().getWindow(SCENE_INDEX_STACK);
						auto& component = scene.getComponentByID(componentID);
						auto& lastPosition = component.template getData<glm::vec2>("LastPosition");
						auto& altPressed = component.template getData<bool>("AltPressed");
						auto diff = coords - lastPosition;
						if (altPressed || (!diff.x && !diff.y))
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
			component.template make<UniqueIdentifier>("FocusID",
				window.registerHandler(EVENT_FOCUS,
					[HOST_INDEX_STACK = component.HOST_INDEX_STACK](auto& event)
					{
						auto& focused = event.template castData<bool>();
						auto& window = Registry::GetSingleton().getWindow(HOST_INDEX_STACK);
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
			auto& window = Registry::GetSingleton().getWindow(component.HOST_INDEX_STACK);
			window.deregisterHandler(EVENT_KEY_PRESS, component.template getData<UniqueIdentifier>("AltPressID"));
			window.deregisterHandler(EVENT_MOUSE_MOVE, component.template getData<UniqueIdentifier>("MouseMoveID"));
			window.deregisterHandler(EVENT_FOCUS, component.template getData<UniqueIdentifier>("FocusID"));
		}
	};
	return info;
};