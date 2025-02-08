#pragma once
#include "../Scene.hpp"
namespace zg::vp
{
  struct VML
  {
    Scene &scene;
    uint32_t mouseMoveID = 0;
    uint32_t focusID = 0;
    VML(Scene &scene);
    ~VML();
    void mouseMoveHandler(glm::vec2 coords);
    void focusHandler(bool focused);
  };
}