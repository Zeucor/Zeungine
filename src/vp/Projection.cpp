#include <zg/vp/Projection.hpp>
#include <zg/Window.hpp>
using namespace zg::vp;
Projection::Projection(Window &window, float fov) : window(window),
                                                    fov(fov),
                                                    nearPlane(0.1f),
                                                    farPlane(100.f)
{
  update();
}
Projection::Projection(Window &window, glm::vec2 orthoSize) : window(window),
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
}