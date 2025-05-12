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
			auto& scene = Registry::getScene(component.HOST_INDEX_STACK);
			auto& window = Registry::getWindow(component.HOST_INDEX_STACK);
			component.template make<UniqueIdentifier>(
				"fID",
				window.addKeyUpdateHandler(
					f,
					[HOST_INDEX_STACK = component.HOST_INDEX_STACK, force]()
					{
						auto& scene = Registry::getScene(HOST_INDEX_STACK);
						auto& window = Registry::getWindow(HOST_INDEX_STACK);
						scene.viewPointer->position.x += scene.viewPointer->direction.x * force * window.deltaTime;
						scene.viewPointer->position.y += scene.viewPointer->direction.y * force * window.deltaTime;
						scene.viewPointer->position.z += scene.viewPointer->direction.z * force * window.deltaTime;
						scene.viewPointer->setDirty();
					}));
			component.template make<UniqueIdentifier>(
				"bID",
				window.addKeyUpdateHandler(
					b,
					[HOST_INDEX_STACK = component.HOST_INDEX_STACK, force]()
					{
						auto& scene = Registry::getScene(HOST_INDEX_STACK);
						auto& window = Registry::getWindow(HOST_INDEX_STACK);
						scene.viewPointer->position.x -= scene.viewPointer->direction.x * force * window.deltaTime;
						scene.viewPointer->position.y -= scene.viewPointer->direction.y * force * window.deltaTime;
						scene.viewPointer->position.z -= scene.viewPointer->direction.z * force * window.deltaTime;
						scene.viewPointer->setDirty();
					}));
			component.template make<UniqueIdentifier>(
				"lID",
				window.addKeyUpdateHandler(l,
					[HOST_INDEX_STACK = component.HOST_INDEX_STACK, force]()
					{
						auto& scene = Registry::getScene(HOST_INDEX_STACK);
						auto& window = Registry::getWindow(HOST_INDEX_STACK);
						glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
						glm::vec3 right =
							glm::normalize(glm::cross(scene.viewPointer->direction, worldUp));
						glm::vec3 up =
							glm::normalize(glm::cross(right, scene.viewPointer->direction));
						scene.viewPointer->position.x -= right.x * force * window.deltaTime;
						scene.viewPointer->position.y -= right.y * force * window.deltaTime;
						scene.viewPointer->position.z -= right.z * force * window.deltaTime;
						scene.viewPointer->setDirty();
					}));
			component.template make<UniqueIdentifier>(
				"rID",
				window.addKeyUpdateHandler(r,
					[HOST_INDEX_STACK = component.HOST_INDEX_STACK, force]()
					{
						auto& scene = Registry::getScene(HOST_INDEX_STACK);
						auto& window = Registry::getWindow(HOST_INDEX_STACK);
						glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
						glm::vec3 right =
							glm::normalize(glm::cross(scene.viewPointer->direction, worldUp));
						glm::vec3 up =
							glm::normalize(glm::cross(right, scene.viewPointer->direction));
						scene.viewPointer->position.x += right.x * force * window.deltaTime;
						scene.viewPointer->position.y += right.y * force * window.deltaTime;
						scene.viewPointer->position.z += right.z * force * window.deltaTime;
						scene.viewPointer->setDirty();
					}));
		},
		.onDetachedFunction = [](auto& component) {
			auto& window = Registry::getWindow(component.HOST_INDEX_STACK);
			auto& f = component.template getData<char>("f");
			auto& b = component.template getData<char>("b");
			auto& l = component.template getData<char>("l");
			auto& r = component.template getData<char>("r");
			window.removeKeyUpdateHandler(f, component.template getData<UniqueIdentifier>("fID"));
			window.removeKeyUpdateHandler(b, component.template getData<UniqueIdentifier>("bID"));
			window.removeKeyUpdateHandler(l, component.template getData<UniqueIdentifier>("lID"));
			window.removeKeyUpdateHandler(r, component.template getData<UniqueIdentifier>("rID"));
		}
	};
	return info;
}
