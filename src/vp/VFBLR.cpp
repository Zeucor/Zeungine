#include <zg/vp/VFBLR.hpp>
using namespace zg::vp;
VFBLR::VFBLR(Scene& scene, KeyScheme keyScheme, float force):
    scene(scene),
    keyScheme(keyScheme),
    force(force)
{
    f = keyScheme == KeyScheme::UDLRSH ? KEYCODE_UP : 'w';
    b = keyScheme == KeyScheme::UDLRSH ? KEYCODE_DOWN : 's';
    l = keyScheme == KeyScheme::UDLRSH ? KEYCODE_LEFT : 'a';
    r = keyScheme == KeyScheme::UDLRSH ? KEYCODE_RIGHT : 'd';
    std::function<void()> onFrontTickFunction = std::bind(&VFBLR::onFrontTick, this);
    fID = scene.window.addKeyUpdateHandler(f, onFrontTickFunction);
    std::function<void()> onBackTickFunction = std::bind(&VFBLR::onBackTick, this);
    bID = scene.window.addKeyUpdateHandler(b, onBackTickFunction);
    std::function<void()> onLeftTickFunction = std::bind(&VFBLR::onLeftTick, this);
    lID = scene.window.addKeyUpdateHandler(l, onLeftTickFunction);
    std::function<void()> onRightTickFunction = std::bind(&VFBLR::onRightTick, this);
    rID = scene.window.addKeyUpdateHandler(r, onRightTickFunction);
}
VFBLR::~VFBLR()
{
    scene.window.removeKeyUpdateHandler(f, fID);
    scene.window.removeKeyUpdateHandler(b, bID);
    scene.window.removeKeyUpdateHandler(l, lID);
    scene.window.removeKeyUpdateHandler(r, rID);
}
void VFBLR::onFrontTick()
{
    scene.view.position.x += scene.view.direction.x * force * scene.window.deltaTime;
    scene.view.position.y += scene.view.direction.y * force * scene.window.deltaTime;
    scene.view.position.z += scene.view.direction.z * force * scene.window.deltaTime;
}
void VFBLR::onBackTick()
{
    scene.view.position.x -= scene.view.direction.x * force * scene.window.deltaTime;
    scene.view.position.y -= scene.view.direction.y * force * scene.window.deltaTime;
    scene.view.position.z -= scene.view.direction.z * force * scene.window.deltaTime;
}
void VFBLR::onLeftTick()
{
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(scene.view.direction, worldUp));
    glm::vec3 up = glm::normalize(glm::cross(right, scene.view.direction));
    scene.view.position.x -= right.x * force * scene.window.deltaTime;
    scene.view.position.y -= right.y * force * scene.window.deltaTime;
    scene.view.position.z -= right.z * force * scene.window.deltaTime;
}
void VFBLR::onRightTick()
{
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(scene.view.direction, worldUp));
    glm::vec3 up = glm::normalize(glm::cross(right, scene.view.direction));
    scene.view.position.x += right.x * force * scene.window.deltaTime;
    scene.view.position.y += right.y * force * scene.window.deltaTime;
    scene.view.position.z += right.z * force * scene.window.deltaTime;
}