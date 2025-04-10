#include <zg/Window.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
#include <zg/components/scenes/GravityByVector.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/vp/VML.hpp>
#include <zg/vp/VFBLR.hpp>
struct PhysicsScene : zg::Scene
{
    zg::vp::VML vml;
    zg::vp::VFBLR vfblr;
    std::shared_ptr<zg::entities::Cube> floor;
    std::shared_ptr<zg::entities::Cube> cube;
    std::shared_ptr<zg::components::entities::RigidBody> cubeRigidBody;
    zg::UniqueIdentifier fID = 0;
    zg::UniqueIdentifier bID = 0;
    zg::UniqueIdentifier lID = 0;
    zg::UniqueIdentifier rID = 0;
    zg::UniqueIdentifier sID = 0;
    int f = 0;
    int b = 0;
    int l = 0;
    int r = 0;
    int s = 0;
    PhysicsScene(zg::Window& window):
        Scene(window, { 50, 50, 50 }, { 0, -1, 1 }, 81.f),
        vml(*this),
        vfblr(*this, zg::vp::VFBLR::KeyScheme::WSADSC, 8.f)
    {
        clearColor = {0, 0, 1, 1};
        addComponent(std::make_shared<zg::components::scenes::GravityByVector>(glm::vec3(0, -2.345, 0)));
        addComponent(std::make_shared<zg::components::scenes::PhysicsScene>(*this));
        floor = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 40, 50), glm::vec3(0), glm::vec3(1), glm::vec3(20, 0.5, 20));
        floor->addComponent(std::make_shared<zg::components::entities::RigidBody>(zg::components::entities::RigidBodyInfo{*floor, zg::components::entities::BodyType::Static}));
        floor->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
            *floor,
            std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(20, 0.5, 20) / 2.f),
            zg::components::entities::PhysicsMaterial{},
            glm::vec3(0),
            glm::quat(1, 0, 0, 0),
            false
        }));
        addEntity(floor);
        cube = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 47, 58), glm::vec3(0), glm::vec3(1), glm::vec3(1.5, 1.5, 1.5));
        cubeRigidBody = std::make_shared<zg::components::entities::RigidBody>(zg::components::entities::RigidBodyInfo{*cube, zg::components::entities::BodyType::Dynamic});
        cube->addComponent(cubeRigidBody);
        cube->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
            *cube,
            // std::make_shared<zg::components::entities::MeshShapeData>(*cube),
            std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1.5, 1.5, 1.5) / 2.f),
            zg::components::entities::PhysicsMaterial{0.5f, 0.8f},
            glm::vec3(0),
            glm::quat(1, 0, 0, 0),
            false
        }));
        addEntity(cube);
        f = KEYCODE_UP;
        b = KEYCODE_DOWN;
        l = KEYCODE_LEFT;
        r = KEYCODE_RIGHT;
        s = 32;
        std::function<void()> onFrontTickFunction = [&]()
        {
            cubeRigidBody->applyForceToCenter({0, 0, 10});
        };
        fID = window.addKeyUpdateHandler(f, onFrontTickFunction);
        std::function<void()> onBackTickFunction = [&]()
        {
            cubeRigidBody->applyForceToCenter({0, 0, -5});
        };
        bID = window.addKeyUpdateHandler(b, onBackTickFunction);
        std::function<void()> onLeftTickFunction = [&]()
        {
            cubeRigidBody->applyForceToCenter({5, 0, 0});
        };
        lID = window.addKeyUpdateHandler(l, onLeftTickFunction);
        std::function<void()> onRightTickFunction = [&]()
        {
            cubeRigidBody->applyForceToCenter({-5, 0, 0});
        };
        rID = window.addKeyUpdateHandler(r, onRightTickFunction);
        std::function<void()> onSpaceTickFunction = [&]()
        {
            cubeRigidBody->applyForceToCenter({0, 5, 0});
        };
        sID = window.addKeyUpdateHandler(s, onSpaceTickFunction);
    }
    ~PhysicsScene()
    {
        window.removeKeyUpdateHandler(f, fID);
        window.removeKeyUpdateHandler(b, bID);
        window.removeKeyUpdateHandler(l, lID);
        window.removeKeyUpdateHandler(r, rID);
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