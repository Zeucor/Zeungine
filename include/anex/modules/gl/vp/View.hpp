#pragma once
#include <anex/glm.hpp>
namespace anex::modules::gl::vp
{
	struct View
  {
    glm::vec3 position;
    glm::vec3 direction;
    glm::mat4 matrix;
    float phi;
    float theta;
    View(const glm::vec3& position, const glm::vec3& direction);
    void update();
		void addPhiTheta(const float &addPhi, const float &addTheta);
  };
}