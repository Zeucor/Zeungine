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
    if (window.iRenderer->renderer == RENDERER_VULKAN)
    {
      matrix = glm::infinitePerspectiveRH_ZO(glm::radians(fov), (const float&)window.windowWidth / (const float&)window.windowHeight, nearPlane);
    }
    else
    {
      matrix = glm::infinitePerspectiveRH_NO(glm::radians(fov), (const float&)window.windowWidth / (const float&)window.windowHeight, nearPlane);
    }
  }
  else
  {
    if (window.iRenderer->renderer == RENDERER_VULKAN)
    {
			matrix = glm::orthoRH_ZO(-orthoSize.x / 2, orthoSize.x / 2, -orthoSize.y / 2, orthoSize.y / 2, nearPlane, farPlane);
    }
    else
    {
			matrix = glm::orthoRH_NO(-orthoSize.x / 2, orthoSize.x / 2, -orthoSize.y / 2, orthoSize.y / 2, nearPlane, farPlane);
    }
  }
  if (window.iRenderer->renderer == RENDERER_VULKAN)
    matrix[1][1] *= -1.0f;
  inverseMatrix = glm::inverse(matrix);
}
template<>
Serial& serialize(Serial& serial, const std::shared_ptr<zg::vp::Projection>& projectionPointer)
{
	auto& projection = *projectionPointer;
	serial << true;
  serial << projection.isOrthographic;
  if (projection.isOrthographic)
  {
    serial << projection.orthoSize;
  }
  else
  {
    serial << projection.fov;
  }
  serial << projection.nearPlane << projection.farPlane;
	return serial;
}
template<>
Serial& deserialize(Serial& serial, std::shared_ptr<zg::vp::Projection>& projectionPointer)
{
	bool wroteBit = false;
	serial >> wroteBit;
	if (!wroteBit)
		return serial;
	bool isOrthographic = false;
  serial >> isOrthographic;
  glm::vec2 orthoSize{0};
  float fov = 0;
  float nearPlane = 0;
  float farPlane = 0;
  if (isOrthographic)
  {
    serial >> orthoSize;
  }
  else
  {
    serial >> fov;
  }
  serial >> nearPlane >> farPlane;
  auto& window = *(zg::Window*)serial.getContextPointer("Window");
  if (isOrthographic)
  {
    projectionPointer = std::make_shared<zg::vp::Projection>(window, orthoSize, nearPlane, farPlane);
  }
  else
  {
    projectionPointer = std::make_shared<zg::vp::Projection>(window, fov, nearPlane, farPlane);
  }
	return serial;
}