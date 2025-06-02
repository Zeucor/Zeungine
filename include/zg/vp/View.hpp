#pragma once
#include <zg/glm.hpp>
#include <zg/Events.hpp>
#include <map>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <zg/EventExecutor.hpp>
namespace zg::vp
{
  struct View : EventExecutor
  {
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 up;
    glm::mat4 matrix;
    float phi;
    float theta;
    bool lookAtSet = false;
    glm::vec3 lookAt = glm::vec3(0);
    float accumulatedPhi;
    float accumulatedTheta;
    bool running = true;
    View(glm::vec3 position, glm::vec3 direction, glm::vec3 up);
    View(glm::vec3 position, glm::vec3 direction, glm::vec3 up, bool lookAtSet, glm::vec3 lookAt);
    void update();
    void addPhiTheta(float addPhi, float addTheta);
    void setDirty();
  };
}