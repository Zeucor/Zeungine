#pragma once
#include <zg/glm.hpp>
#include <zg/Events.hpp>
#include <map>
namespace zg::vp
{
  struct View
  {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
    glm::mat4 matrix;
    float phi;
    float theta;
    std::pair<UniqueIdentifier, std::map<UniqueIdentifier, ViewResizeHandler>> viewResizeHandlers;
    bool lookAtSet = false;
    glm::vec3 lookAt = glm::vec3(0);
    View(glm::vec3 position, glm::vec3 direction, glm::vec3 up);
    View(glm::vec3 position, glm::vec3 direction, glm::vec3 up, bool lookAtSet, glm::vec3 lookAt);
    void update();
    void addPhiTheta(float addPhi, float addTheta);
    UniqueIdentifier addResizeHandler(const ViewResizeHandler &callback);
    void removeResizeHandler(UniqueIdentifier &id);
    void callResizeHandler(glm::vec2 newSize);
  };
}