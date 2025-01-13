#define RUNTIME_EXPORTS
#include <anex/RuntimeAPI.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/entities/Cube.hpp>
using namespace anex;
struct EditorScene : GLScene
{
  EditorScene(GLWindow &window):
    GLScene(window, {0, 10, 10}, {0, -1, -1}, 81.f)
  {
    addEntity(std::make_shared<entities::Cube>(window, *this, glm::vec3(0, 1, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::vec3(1, 1, 1)));
    addEntity(std::make_shared<entities::Cube>(window, *this, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::vec3(25, 1, 25)));
  };
};
RUNTIME_EXPORT void Load(GLWindow &window)
{
  window.clearColor = {0, 0, 1, 1};
  window.setIScene(std::make_shared<EditorScene>(window));
};