#include <zg/Window.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/RigidBodyCollider.hpp>
#include <zg/components/scenes/Physics.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/vp/VML.hpp>
#include <zg/vp/VFBLR.hpp>
struct PhysicsScene : zg::Scene
{
    zg::vp::VML vml;
    zg::vp::VFBLR vfblr;
    std::shared_ptr<zg::entities::Cube> floor;
    std::shared_ptr<zg::entities::Cube> cube;
    PhysicsScene(zg::Window& window):
        Scene(window, { 50, 50, 50 }, { 0, 0, 1 }, 81.f),
        vml(*this),
        vfblr(*this, zg::vp::VFBLR::KeyScheme::WSADSC, 8.f)
    {
        clearColor = {0, 0, 1, 1};
        addComponent(std::make_shared<zg::components::scenes::Physics>(*this));
        floor = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 40, 50), glm::vec3(0), glm::vec3(1), glm::vec3(20, 0.5, 20));
        floor->addComponent(std::make_shared<zg::components::entities::RigidBodyCollider>(*floor, true));
        addEntity(floor);
        cube = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 47, 58), glm::vec3(0), glm::vec3(1), glm::vec3(1.5, 1.5, 1.5));
        cube->addComponent(std::make_shared<zg::components::entities::RigidBodyCollider>(*cube, false));
        addEntity(cube);
    }
};
int main()
{
    zg::Window window("Physics Test", 1024, 768, -1, -1, false, false, 60);
    window.runOnThread([](auto& window)
    {
        window.setScene(std::make_shared<PhysicsScene>(window));
    });
    window.addKeyPressHandler(27, [&](auto pressed)
    {
        if (pressed)
            window.close();
    });
    window.run();
    return 0;
}