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
    glm::mat4 matrix;
    float phi;
    float theta;
    std::pair<UniqueIdentifier, std::map<UniqueIdentifier, ViewResizeHandler>> viewResizeHandlers;
    View(glm::vec3 position, glm::vec3 direction);
    void update();
    void addPhiTheta(float addPhi, float addTheta);
    UniqueIdentifier addResizeHandler(const ViewResizeHandler &callback);
    void removeResizeHandler(UniqueIdentifier &id);
    void callResizeHandler(glm::vec2 newSize);
  };
}