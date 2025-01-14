#define RUNTIME_EXPORTS
#include <anex/RuntimeAPI.hpp>
#include <anex/modules/gl/GLScene.hpp>
#include <anex/modules/gl/entities/Cube.hpp>
#include <anex/modules/gl/entities/TextView.hpp>
#include <anex/modules/gl/entities/SkyBox.hpp>
#include <anex/modules/gl/vp/VML.hpp>
#include <anex/modules/gl/fonts/freetype/Freetype.hpp>
using namespace anex;
struct EditorScene : GLScene
{
  vp::VML vml;
  File robotoRegularFile;
  std::shared_ptr<textures::Texture> texturePointer;
  modules::gl::fonts::freetype::FreetypeFont robotoRegularFont;
  EditorScene(GLWindow &window):
    GLScene(window, {0, 10, 10}, {0, -1, -1}, 81.f),
    vml(*this),
    robotoRegularFile("fonts/Roboto/Roboto-Regular.ttf", enums::EFileLocation::Relative, "r"),
    robotoRegularFont(window, robotoRegularFile)
  {
    addEntity(std::make_shared<entities::Cube>(window, *this, glm::vec3(0, 1, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::vec3(1, 1, 1)));
    addEntity(std::make_shared<entities::Cube>(window, *this, glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), glm::vec3(25, 1, 25)));
    // addEntity()
    addEntity(std::make_shared<entities::SkyBox>(
      (GLWindow &)window,
      *this,
      std::vector<std::string_view>({
        "images/skybox/right.jpg", "images/skybox/left.jpg", "images/skybox/top.jpg",
        "images/skybox/bottom.jpg", "images/skybox/front.jpg", "images/skybox/back.jpg"
      })
    ));
    addEntity(std::make_shared<entities::TextView>(window, *this, glm::vec3(0, 5, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), "Test Text", glm::vec2( 47 / 3, 14 / 3), robotoRegularFont, 90.f));
  };
};
RUNTIME_EXPORT void Load(GLWindow &window)
{
  auto scene = std::make_shared<EditorScene>(window);
  scene->clearColor = {0, 0, 1, 1};
  window.setIScene(scene);
};