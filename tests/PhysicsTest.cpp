#include <zg/Window.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/RigidBodyCollider.hpp>
#include <zg/components/scenes/Physics.hpp>
struct PhysicsScene : zg::Scene
{
    PhysicsScene(zg::Window& window):
        Scene(window, { 50, 50, 50 }, { 0, 0, 1 }, 81.f)
    {
        addComponent(std::make_shared<zg::components::scenes::Physics>(*this));
    }
};
int main()
{
    zg::Window window("Physics Test", 1024, 768, -1, -1, false, false, 60);
    window.runOnThread([](auto& window)
    {
        window.setScene(std::make_shared<PhysicsScene>(window));
    });
    window.run();
    return 0;
}