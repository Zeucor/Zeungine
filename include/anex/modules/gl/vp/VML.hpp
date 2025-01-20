#pragma once
#include "../GLScene.hpp"
namespace anex::modules::gl::vp
{
	struct VML
  {
    GLScene &scene;
    uint32_t mouseMoveID = 0;
    VML(GLScene &scene);
    ~VML();
    void mouseMoveHandler(glm::vec2 coords);
  };
}