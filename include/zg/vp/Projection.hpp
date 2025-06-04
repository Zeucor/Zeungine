#pragma once
#include <zg/glm.hpp>
namespace zg
{
  struct Window;
}
namespace zg::vp
{
  struct Projection
  {
    enum TYPE
    {
      Perspective,
      Orthographic
    };
    Window &window;
    float fov = 0;
    glm::vec2 orthoSize = glm::vec2(0.0f);
    bool isOrthographic = false;
    float nearPlane;
    float farPlane;
    glm::mat4 matrix;
    glm::mat4 inverseMatrix;
    Projection(Window &window, float fov, float nearPlane = 0.1f, float farPlane = 100.f);
    Projection(Window &window, glm::vec2 orthoSize, float nearPlane = 0.1f, float farPlane = 100.f);
    void update();
  };
}