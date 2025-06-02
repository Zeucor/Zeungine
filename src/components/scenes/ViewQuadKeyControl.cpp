#include <zg/Registry.hpp>
#include <zg/Window.hpp>
#include <zg/components/scenes/ViewQuadKeyControl.hpp>
using namespace zg;
using namespace zg::vp;
components::scenes::SceneComponentCreateInfo components::scenes::ViewQuadKeyControlFactory(KeyScheme keyScheme,
																																																	 float force)
{
	components::scenes::SceneComponentCreateInfo info{
		.name = "View Quad Key Control",
		.onAttachedFunction = [keyScheme, force](auto& component)
		{
			auto& f = (component.template make<char>("f", keyScheme == KeyScheme::UDLRSH ? KEYCODE_UP : 'w'));
			auto& b = (component.template make<char>("b", keyScheme == KeyScheme::UDLRSH ? KEYCODE_DOWN : 's'));
			auto& l = (component.template make<char>("l", keyScheme == KeyScheme::UDLRSH ? KEYCODE_LEFT : 'a'));
			auto& r = (component.template make<char>("r", keyScheme == KeyScheme::UDLRSH ? KEYCODE_RIGHT : 'd'));
			std::unordered_map<char, glm::vec3> keyDirections = {
				{ f, {0, 0, 1} },
				{ b, {0, 0,-1} },
				{ l, {-1,0, 0} },
				{ r, {1, 0, 0} }
			};
			component.template make<std::unordered_map<char, bool>>("KeysPressed");
			component.template make<glm::vec3>("CurrentDirection", glm::vec3(0));
			auto& scene = Registry::GetSingleton().getScene(component.HOST_INDEX_STACK);
			auto& window = Registry::GetSingleton().getWindow(component.HOST_INDEX_STACK);
			component.template make<size_t>(
				"KeyID",
				window.registerHandler(EVENT_KEY_PRESS, [
					keyDirections,
					f, b, l, r,
					HOST_INDEX_STACK = component.HOST_INDEX_STACK,
					component_ID = component.ID
				](auto& event) {
					auto key = event.getValue();
					if (key != f && key != b && key != l && key != r)
						return;
					auto& pressed = event.template castData<bool>();
					auto& rgy = Registry::GetSingleton();
					auto& scene = rgy.getScene(HOST_INDEX_STACK);
					auto& component = scene.getComponentByID(component_ID);
					auto& keysPressed = component.template getData<std::unordered_map<char, bool>>("KeysPressed");
					auto& keyPressed = keysPressed[key];
					if (keyPressed != pressed)
					{
						auto& currentDirection = component.template getData<glm::vec3>("CurrentDirection");
						auto addDirection_iter = keyDirections.find(key);
						if (addDirection_iter == keyDirections.end())
							return;
						auto addDirection = addDirection_iter->second;
						if (!pressed)
							addDirection *= -1.f;
						currentDirection += addDirection;
						keyPressed = pressed;
					}
				})
			);
		},
		.onDetachedFunction = [](auto& component) {
			auto& window = Registry::GetSingleton().getWindow(component.HOST_INDEX_STACK);
			window.deregisterHandler(EVENT_KEY_PRESS, component.template getData<size_t>("KeyID"));
		},
		.onUpdateFunction = [force](auto& component) {
			auto& rgy = Registry::GetSingleton();
			auto& scene = rgy.getScene(component.HOST_INDEX_STACK);
			auto& window = rgy.getWindow(component.HOST_INDEX_STACK);
			auto& currentDirection = component.template getData<glm::vec3>("CurrentDirection");
			auto& view = *scene.viewPointer;
			auto& viewDirection = view.direction;
			auto& viewUp = view.up;
			auto& viewPosition = view.position;
			glm::vec3 forward = glm::normalize(viewDirection);
			glm::vec3 right = glm::normalize(glm::cross(forward, viewUp));
			glm::vec3 up = glm::normalize(viewUp);
			glm::vec3 calculatedDirection = 
				currentDirection.x * right + 
				currentDirection.y * up + 
				currentDirection.z * forward;
			viewPosition += calculatedDirection * force * ((float)((const long double&)window.lastFrameDeltaTime));
			view.setDirty();
		}
	};
	return info;
}
