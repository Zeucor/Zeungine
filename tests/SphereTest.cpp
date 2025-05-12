#include <zg/Window.hpp>
#include <zg/Registry.hpp>
#include <zg/entities/Sphere.hpp>
#include <zg/components/scenes/ViewMouseControl.hpp>
#include <zg/components/scenes/ViewQuadKeyControl.hpp>
using namespace zg;
SceneCreateInfo SphereSceneFactory();
int main()
{
    WindowCreateInfo windowInfo{
        .title = "Sphere Test",
        .windowWidth = 1920,
        .windowHeight = 1080,
        .windowX = 0,
        .windowY = 0,
        .borderless = true,
        .vsync = false,
        .framerate = 88
    };
    auto window_tuple = Registry::addWindow(windowInfo);
    auto& window = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(window_tuple);
    window.runOnThread([](auto& window){
        window.addScene(SphereSceneFactory());
    });
    window.addKeyPressHandler('q', [&](auto pressed){
        if (!pressed)
            return;
        window.close();
    });
    window.run();
}
SceneCreateInfo SphereSceneFactory()
{
    SceneCreateInfo info{
        .name = "Sphere Scene",
        .cameraPosition = {5, 5, 5},
        .cameraDirection = {-1, -1, -1},
        .fov = 81,
        .onAttachedFunction = [](auto& scene){
            scene.clearColor = {0.4, 0.3, 0.5, 1.0};
            auto sphereInfo = entities::SphereFactory(glm::vec4(0.5, 0.7, 0.15, 1.0), "Sphery");
            scene.addEntity(sphereInfo);
            scene.attachComponent(components::scenes::ViewMouseControlFactory());
            scene.attachComponent(components::scenes::ViewQuadKeyControlFactory(components::scenes::KeyScheme::WSADSC, 18));
        },
        .onDetachedFunction = [](auto& scene){

        }
    };
    return info;
}