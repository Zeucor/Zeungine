#pragma once
#include <anex/glm.hpp>
namespace anex::modules::gl
{
	struct GLWindow;
}
namespace anex::modules::gl::vp
{
	struct Projection
  {
    GLWindow &window;
    float fov = 0;
    glm::vec2 orthoSize = glm::vec2(0.0f);
    bool isOrthographic = false;
    glm::mat4 matrix = glm::mat4(1.0f);
    Projection(GLWindow &window, const float &fov);
		Projection(GLWindow &window, const glm::vec2 &orthoSize);
    void update();
  };
}