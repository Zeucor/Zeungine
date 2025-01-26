#include <zg/modules/gl/vp/Projection.hpp>
#include <zg/modules/gl/RenderWindow.hpp>
using namespace zg::modules::gl::vp;
Projection::Projection(RenderWindow &window, float fov):
  window(window),
	fov(fov),
  nearPlane(0.1f),
  farPlane(100.f)
{
  update();
};
Projection::Projection(RenderWindow &window, glm::vec2 orthoSize):
  window(window),
  orthoSize(orthoSize),
  isOrthographic(true),
  nearPlane(0.1f),
  farPlane(100.f)
{
  update();
}
void Projection::update()
{
  if (!isOrthographic)
  	matrix = glm::perspective(glm::radians(fov), (float)window.windowWidth / window.windowHeight, nearPlane, farPlane);
  else
    matrix = glm::ortho(-orthoSize.x / 2, orthoSize.x / 2, -orthoSize.y / 2, orthoSize.y / 2, nearPlane, farPlane);
};