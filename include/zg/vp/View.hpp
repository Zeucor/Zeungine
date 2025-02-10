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
    std::pair<EventIdentifier, std::map<EventIdentifier, ViewResizeHandler>> viewResizeHandlers;
    View(glm::vec3 position, glm::vec3 direction);
    void update();
    void addPhiTheta(float addPhi, float addTheta);
    EventIdentifier addResizeHandler(const ViewResizeHandler &callback);
    void removeResizeHandler(EventIdentifier &id);
    void callResizeHandler(glm::vec2 newSize);
  };
}