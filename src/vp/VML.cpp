#include <zg/vp/VML.hpp>
#include <zg/interfaces/IWindow.hpp>
using namespace zg::vp;
VML::VML(Scene &scene) : scene(scene)
{
  auto &window = scene.window;
  mouseMoveID = window.addMouseMoveHandler(std::bind(&VML::mouseMoveHandler, this, std::placeholders::_1));
  focusID = window.addFocusHandler(std::bind(&VML::focusHandler, this, std::placeholders::_1));
};
VML::~VML()
{
  scene.window.removeMouseMoveHandler(mouseMoveID);
  scene.window.removeFocusHandler(focusID);
};
void VML::mouseMoveHandler(glm::vec2 coords)
{
  if (scene.window.justWarpedPointer)
  {
    scene.window.justWarpedPointer = false;
    return;
  }
  if (!scene.window.focused)
    return;
  glm::vec2 center = {scene.window.windowWidth / 2, scene.window.windowHeight / 2};
  auto currentWindow = (Window &)scene.window;
  auto diff = coords - center;
  scene.view.addPhiTheta(diff.x * 0.001f, -diff.y * 0.001f);
  scene.window.warpPointer(center);
}
void VML::focusHandler(bool focused)
{
  if (focused)
    scene.window.iPlatformWindow->hidePointer();
  else
    scene.window.iPlatformWindow->showPointer();
}