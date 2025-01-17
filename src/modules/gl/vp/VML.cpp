#include <anex/modules/gl/vp/VML.hpp>
#include <anex/IWindow.hpp>
using namespace anex::modules::gl::vp;
VML::VML(GLScene &scene):
	scene(scene)
{
  auto &window = scene.window;
  mouseMoveID = window.addMouseMoveHandler(std::bind(&VML::mouseMoveHandler, this, std::placeholders::_1));
};
VML::~VML()
{
  scene.window.removeMouseMoveHandler(mouseMoveID);
};
void VML::mouseMoveHandler(const glm::vec2 &coords)
{
  if (scene.window.justWarpedPointer)
  {
    scene.window.justWarpedPointer = false;
    return;
  }
  if (!scene.window.focused)
    return;
  glm::vec2 center = {scene.window.windowWidth/2, scene.window.windowHeight/2};
  auto currentWindow = (GLWindow &)scene.window;
  auto diff = coords - center;
  scene.view.addPhiTheta(diff.x * 0.001f, diff.y * (currentWindow.isChildWindow ? 1.f : -1.f) * 0.001f);
  scene.window.warpPointer(center);
};