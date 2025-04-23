#include <zg/Registry.hpp>
#include <zg/Window.hpp>
#include <zg/components/scenes/ViewQuadKeyControl.hpp>
using namespace zg::vp;
zg::components::scenes::SceneComponentCreateInfo zg::components::scenes::ViewQuadKeyControlFactory(KeyScheme keyScheme,
																																																	 float force)
{
	zg::components::scenes::SceneComponentCreateInfo info{
		.name = "View Quad Key Control",
		.onAttachedFunction = [keyScheme, force](auto& component)
		{
			auto& f = (component.template make<char>("f", keyScheme == KeyScheme::UDLRSH ? KEYCODE_UP : 'w'));
			auto& b = (component.template make<char>("b", keyScheme == KeyScheme::UDLRSH ? KEYCODE_DOWN : 's'));
			auto& l = (component.template make<char>("l", keyScheme == KeyScheme::UDLRSH ? KEYCODE_LEFT : 'a'));
			auto& r = (component.template make<char>("r", keyScheme == KeyScheme::UDLRSH ? KEYCODE_RIGHT : 'd'));
			auto& scene = zg::Registry::getScene(component.hostIndexStack);
			component.template make<zg::UniqueIdentifier>(
				"fID",
				scene.window.addKeyUpdateHandler(
					f,
					[hostIndexStack = component.hostIndexStack, force]()
					{
						auto& scene = zg::Registry::getScene(hostIndexStack);
						scene.viewPointer->position.x += scene.viewPointer->direction.x * force * scene.window.deltaTime;
						scene.viewPointer->position.y += scene.viewPointer->direction.y * force * scene.window.deltaTime;
						scene.viewPointer->position.z += scene.viewPointer->direction.z * force * scene.window.deltaTime;
						scene.viewPointer->setDirty();
					}));
			component.template make<zg::UniqueIdentifier>(
				"bID",
				scene.window.addKeyUpdateHandler(
					b,
					[hostIndexStack = component.hostIndexStack, force]()
					{
						auto& scene = zg::Registry::getScene(hostIndexStack);
						scene.viewPointer->position.x -= scene.viewPointer->direction.x * force * scene.window.deltaTime;
						scene.viewPointer->position.y -= scene.viewPointer->direction.y * force * scene.window.deltaTime;
						scene.viewPointer->position.z -= scene.viewPointer->direction.z * force * scene.window.deltaTime;
						scene.viewPointer->setDirty();
					}));
			component.template make<zg::UniqueIdentifier>(
				"lID",
				scene.window.addKeyUpdateHandler(l,
																				 [hostIndexStack = component.hostIndexStack, force]()
																				 {
																					 auto& scene = zg::Registry::getScene(hostIndexStack);
																					 glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
																					 glm::vec3 right =
																						 glm::normalize(glm::cross(scene.viewPointer->direction, worldUp));
																					 glm::vec3 up =
																						 glm::normalize(glm::cross(right, scene.viewPointer->direction));
																					 scene.viewPointer->position.x -= right.x * force * scene.window.deltaTime;
																					 scene.viewPointer->position.y -= right.y * force * scene.window.deltaTime;
																					 scene.viewPointer->position.z -= right.z * force * scene.window.deltaTime;
																					 scene.viewPointer->setDirty();
																				 }));
			component.template make<zg::UniqueIdentifier>(
				"rID",
				scene.window.addKeyUpdateHandler(r,
																				 [hostIndexStack = component.hostIndexStack, force]()
																				 {
																					 auto& scene = zg::Registry::getScene(hostIndexStack);
																					 glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
																					 glm::vec3 right =
																						 glm::normalize(glm::cross(scene.viewPointer->direction, worldUp));
																					 glm::vec3 up =
																						 glm::normalize(glm::cross(right, scene.viewPointer->direction));
																					 scene.viewPointer->position.x += right.x * force * scene.window.deltaTime;
																					 scene.viewPointer->position.y += right.y * force * scene.window.deltaTime;
																					 scene.viewPointer->position.z += right.z * force * scene.window.deltaTime;
																					 scene.viewPointer->setDirty();
																				 }));
		}};
	return info;
}
