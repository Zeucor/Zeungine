#include <zg/vp/Projection.hpp>
#include <zg/Window.hpp>
using namespace zg::vp;
Projection::Projection(Window &window, float fov, float nearPlane, float farPlane) : window(window),
                                                    fov(fov),
                                                    nearPlane(0.1f),
                                                    farPlane(100.f)
{
  update();
}
Projection::Projection(Window &window, glm::vec2 orthoSize, float nearPlane, float farPlane) : window(window),
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
  {
    if (window.iRenderer->renderer == RENDERER::RENDERER_VULKAN)
    {
      matrix = glm::infinitePerspectiveRH_ZO(glm::radians(fov), (float)window.windowWidth / window.windowHeight, nearPlane);
    }
    else
    {
      matrix = glm::infinitePerspectiveRH_NO(glm::radians(fov), (float)window.windowWidth / window.windowHeight, nearPlane);
    }
  }
  else
  {
    if (window.iRenderer->renderer == RENDERER::RENDERER_VULKAN)
    {
			matrix = glm::orthoRH_ZO(-orthoSize.x / 2, orthoSize.x / 2, -orthoSize.y / 2, orthoSize.y / 2, nearPlane, farPlane);
    }
    else
    {
			matrix = glm::orthoRH_NO(-orthoSize.x / 2, orthoSize.x / 2, -orthoSize.y / 2, orthoSize.y / 2, nearPlane, farPlane);
    }
  }
}