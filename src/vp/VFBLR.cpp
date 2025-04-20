#include <zg/vp/VFBLR.hpp>
#include <zg/Window.hpp>
using namespace zg::vp;
zg::components::scenes::SceneComponentCreateInfo zg::components::scenes::ViewQuadKeyControlFactory(KeyScheme keyScheme, float force)
{
    zg::components::scenes::SceneComponentCreateInfo info{
        .name = "View Quad Key Control",

    };
    return info;
}
// VFBLR::VFBLR(Scene& scene, KeyScheme keyScheme, float force):
    
//     scene(scene),
//     keyScheme(keyScheme),
//     force(force)
// {
//     f = keyScheme == KeyScheme::UDLRSH ? KEYCODE_UP : 'w';
//     b = keyScheme == KeyScheme::UDLRSH ? KEYCODE_DOWN : 's';
//     l = keyScheme == KeyScheme::UDLRSH ? KEYCODE_LEFT : 'a';
//     r = keyScheme == KeyScheme::UDLRSH ? KEYCODE_RIGHT : 'd';
//     std::function<void()> onFrontTickFunction = std::bind(&VFBLR::onFrontTick, this);
//     fID = scene.window.addKeyUpdateHandler(f, onFrontTickFunction);
//     std::function<void()> onBackTickFunction = std::bind(&VFBLR::onBackTick, this);
//     bID = scene.window.addKeyUpdateHandler(b, onBackTickFunction);
//     std::function<void()> onLeftTickFunction = std::bind(&VFBLR::onLeftTick, this);
//     lID = scene.window.addKeyUpdateHandler(l, onLeftTickFunction);
//     std::function<void()> onRightTickFunction = std::bind(&VFBLR::onRightTick, this);
//     rID = scene.window.addKeyUpdateHandler(r, onRightTickFunction);
// }
// VFBLR::~VFBLR()
// {
//     scene.window.removeKeyUpdateHandler(f, fID);
//     scene.window.removeKeyUpdateHandler(b, bID);
//     scene.window.removeKeyUpdateHandler(l, lID);
//     scene.window.removeKeyUpdateHandler(r, rID);
// }
// void VFBLR::onFrontTick()
// {
//     scene.viewPointer->position.x += scene.viewPointer->direction.x * force * scene.window.deltaTime;
//     scene.viewPointer->position.y += scene.viewPointer->direction.y * force * scene.window.deltaTime;
//     scene.viewPointer->position.z += scene.viewPointer->direction.z * force * scene.window.deltaTime;
//     scene.viewPointer->update();
// }
// void VFBLR::onBackTick()
// {
//     scene.viewPointer->position.x -= scene.viewPointer->direction.x * force * scene.window.deltaTime;
//     scene.viewPointer->position.y -= scene.viewPointer->direction.y * force * scene.window.deltaTime;
//     scene.viewPointer->position.z -= scene.viewPointer->direction.z * force * scene.window.deltaTime;
//     scene.viewPointer->update();
// }
// void VFBLR::onLeftTick()
// {
//     glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
//     glm::vec3 right = glm::normalize(glm::cross(scene.viewPointer->direction, worldUp));
//     glm::vec3 up = glm::normalize(glm::cross(right, scene.viewPointer->direction));
//     scene.viewPointer->position.x -= right.x * force * scene.window.deltaTime;
//     scene.viewPointer->position.y -= right.y * force * scene.window.deltaTime;
//     scene.viewPointer->position.z -= right.z * force * scene.window.deltaTime;
//     scene.viewPointer->update();
// }
// void VFBLR::onRightTick()
// {
//     glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
//     glm::vec3 right = glm::normalize(glm::cross(scene.viewPointer->direction, worldUp));
//     glm::vec3 up = glm::normalize(glm::cross(right, scene.viewPointer->direction));
//     scene.viewPointer->position.x += right.x * force * scene.window.deltaTime;
//     scene.viewPointer->position.y += right.y * force * scene.window.deltaTime;
//     scene.viewPointer->position.z += right.z * force * scene.window.deltaTime;
//     scene.viewPointer->update();
// }
// template<>
// Serial& serialize(Serial& serial, const std::shared_ptr<zg::vp::VFBLR>& vfblrPointer)
// {
// 	auto& vfblr = *vfblrPointer;
// 	serial << true << vfblr.keyScheme << vfblr.force;
// 	return serial;
// }
// template<>
// Serial& deserialize(Serial& serial, std::shared_ptr<zg::vp::VFBLR>& vfblrPointer)
// {
// 	bool wroteBit = false;
// 	serial >> wroteBit;
// 	if (!wroteBit)
// 		return serial; 
// 	auto* scenePointer = (zg::Scene*)serial.getContextPointer("Scene");
// 	VFBLR::KeyScheme keyScheme = (VFBLR::KeyScheme)0;
//     float force = 0;
// 	serial >> keyScheme >> force;
// 	vfblrPointer = std::make_shared<zg::vp::VFBLR>(*scenePointer, keyScheme, force);
// 	return serial;
// }