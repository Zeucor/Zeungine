#pragma once
#include "../Scene.hpp"
namespace zg::vp
{
  struct VML
  {
    Scene &scene;
    UniqueIdentifier mouseMoveID = 0;
    UniqueIdentifier focusID = 0;
    VML(Scene &scene);
    ~VML();
    void mouseMoveHandler(glm::vec2 coords);
    void focusHandler(bool focused);
  };
}