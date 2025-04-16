#include <zg/Window.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
#include <zg/components/scenes/GravityByVector.hpp>
#include <zg/components/scenes/GravityByAttraction.hpp>
#include <zg/entities/Cube.hpp>
#include <zg/vp/VML.hpp>
#include <zg/vp/VFBLR.hpp>
#include <zg/math/Rotations.hpp>
#include <zg/components/scenes/EntityThirdPersonCamera.hpp>
#include <zg/physics/CollisionMannifold.hpp>
struct PhysicsScene : zg::Scene
{
    std::shared_ptr<zg::components::scenes::EntityThirdPersonCamera> thirdPerson;
    // zg::vp::VML vml;
    // zg::vp::VFBLR vfblr;
    std::shared_ptr<zg::entities::Cube> floor;
    std::shared_ptr<zg::components::entities::RigidBody> floorRigidBody;
    std::shared_ptr<zg::entities::Cube> cube;
    std::shared_ptr<zg::components::entities::RigidBody> cubeRigidBody;
    std::shared_ptr<zg::entities::Cube> cube2;
    std::shared_ptr<zg::components::entities::RigidBody> cube2RigidBody;
    std::shared_ptr<zg::entities::Cube> cube3;
    std::shared_ptr<zg::components::entities::RigidBody> cube3RigidBody;
    std::shared_ptr<zg::entities::Cube> cube4;
    std::shared_ptr<zg::components::entities::RigidBody> cube4RigidBody;
    std::shared_ptr<zg::entities::Cube> wall1;
    std::shared_ptr<zg::components::entities::RigidBody> wall1RigidBody;
    std::shared_ptr<zg::entities::Cube> wall2;
    std::shared_ptr<zg::components::entities::RigidBody> wall2RigidBody;
    std::shared_ptr<zg::entities::Cube> wall3;
    std::shared_ptr<zg::components::entities::RigidBody> wall3RigidBody;
    std::shared_ptr<zg::entities::Cube> wall4;
    std::shared_ptr<zg::components::entities::RigidBody> wall4RigidBody;
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
        Scene(window, { 50, 50, 50 }, { 0, -1, 1 }, 81.f)
        // vml(*this),
        // vfblr(*this, zg::vp::VFBLR::KeyScheme::WSADSC, 8.f)
    {
        clearColor = {0, 0, 1, 1};
        zg::shaders::RuntimeConstants commonShaderConstants({"Lighting", "DirectionalLightShadowMaps", "LightSpacePosition"});

		// auto dldirection = glm::normalize(zg::math::Rotations::Vec3AroundVec3({0, 0, 1}, {0, 0, 0}, {90, 0, 0}));
        // dldirection = glm::normalize(zg::math::Rotations::Vec3AroundVec3(dldirection, {0, 0, 0}, {0, 90, 90}));
		// auto dlup = glm::normalize(zg::math::Rotations::Vec3AroundVec3({0, 1, 0}, {0, 0, 0}, {90, 0, 0}));
        // dlup = glm::normalize(zg::math::Rotations::Vec3AroundVec3(dlup, {0, 0, 0}, {0, 90, 0}));
        glm::vec3 dldirection{1, -1, 1};
        dldirection = glm::normalize(dldirection);
        glm::vec3 dlup{0, 1, 0};
		directionalLights.push_back({
			glm::vec3(20, 80, 20), // position
			dldirection, // direction
			dlup, // up
			glm::vec3(1.f, 1.f, 1.f), // color
			1.f, // intensity,
			1.f, // nearPlane
			364.f, // farPlane
            0.4f // ambientFactor
		});
		auto& dl = directionalLights[0];
		directionalLightShadows.emplace_back(window, directionalLights[0]);

        // addComponent(std::make_shared<zg::components::scenes::GravityByAttraction>(0.000005f));
        addComponent(std::make_shared<zg::components::scenes::GravityByVector>(glm::vec3(0, -9.81, 0)));
        addComponent(std::make_shared<zg::components::scenes::PhysicsScene>(*this));
        //
        floor = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 40, 50), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec3(2000, 0.5, 2000), commonShaderConstants);
        floorRigidBody = std::make_shared<zg::components::entities::RigidBody>(zg::components::entities::RigidBodyInfo{*floor, zg::components::entities::BodyType::Static});
        floorRigidBody->setMass(1000000);
        floor->addComponent(floorRigidBody);
        floor->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
            *floor,
            std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(2000, 0.5, 2000) / 2.f),
            zg::components::entities::PhysicsMaterial{0.80f, 0.7f},
            glm::vec3(0),
            glm::quat(1, 0, 0, 0),
            false
        }));
        addEntity(floor);
        // cube
        cube = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 47, 58), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec3(1.5, 1.5, 1.5), commonShaderConstants);
        cubeRigidBody = std::make_shared<zg::components::entities::RigidBody>(zg::components::entities::RigidBodyInfo{*cube, zg::components::entities::BodyType::Dynamic});
        cube->addComponent(cubeRigidBody);
        cube->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
            *cube,
            // std::make_shared<zg::components::entities::MeshShapeData>(*cube),
            std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1.5, 1.5, 1.5) / 2.f),
            zg::components::entities::PhysicsMaterial{0.80f, 0.7f},
            glm::vec3(0),
            glm::quat(1, 0, 0, 0),
            false
        }));
        addEntity(cube);
        // cube2
        cube2 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(53, 47, 58), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec3(1.5, 1.5, 1.5), commonShaderConstants);
        cube2RigidBody = std::make_shared<zg::components::entities::RigidBody>(zg::components::entities::RigidBodyInfo{*cube2, zg::components::entities::BodyType::Dynamic});
        cube2->addComponent(cube2RigidBody);
        cube2->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
            *cube2,
            // std::make_shared<zg::components::entities::MeshShapeData>(*cube2),
            std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1.5, 1.5, 1.5) / 2.f),
            zg::components::entities::PhysicsMaterial{0.80f, 0.7f},
            glm::vec3(0),
            glm::quat(1, 0, 0, 0),
            false
        }));
        addEntity(cube2);
        // cube3
        cube3 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(47, 47, 58), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec3(1.5, 1.5, 1.5), commonShaderConstants);
        cube3RigidBody = std::make_shared<zg::components::entities::RigidBody>(zg::components::entities::RigidBodyInfo{*cube3, zg::components::entities::BodyType::Dynamic});
        cube3->addComponent(cube3RigidBody);
        cube3->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
            *cube3,
            // std::make_shared<zg::components::entities::MeshShapeData>(*cube3),
            std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1.5, 1.5, 1.5) / 2.f),
            zg::components::entities::PhysicsMaterial{0.80f, 0.7f},
            glm::vec3(0),
            glm::quat(1, 0, 0, 0),
            false
        }));
        addEntity(cube3);
        // cube4
        cube4 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 47, 54), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec3(1.5, 1.5, 1.5), commonShaderConstants);
        cube4RigidBody = std::make_shared<zg::components::entities::RigidBody>(zg::components::entities::RigidBodyInfo{*cube4, zg::components::entities::BodyType::Dynamic});
        cube4->addComponent(cube4RigidBody);
        cube4->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
            *cube4,
            // std::make_shared<zg::components::entities::MeshShapeData>(*cube4),
            std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1.5, 1.5, 1.5) / 2.f),
            zg::components::entities::PhysicsMaterial{0.80f, 0.7f},
            glm::vec3(0),
            glm::quat(1, 0, 0, 0),
            false
        }));
        addEntity(cube4);
        //
        // walls
        wall1 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(40, 43, 50), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec3(1, 5, 20), commonShaderConstants);
        wall1RigidBody = std::make_shared<zg::components::entities::RigidBody>(zg::components::entities::RigidBodyInfo{*wall1, zg::components::entities::BodyType::Static});
        wall1RigidBody->setMass(2);
        wall1->addComponent(wall1RigidBody);
        wall1->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
            *wall1,
            std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1, 5, 20) / 2.f),
            zg::components::entities::PhysicsMaterial{},
            glm::vec3(0),
            glm::quat(1, 0, 0, 0),
            false
        }));
        addEntity(wall1);
        wall2 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(60, 43, 50), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec3(1, 5, 20), commonShaderConstants);
        wall2RigidBody = std::make_shared<zg::components::entities::RigidBody>(zg::components::entities::RigidBodyInfo{*wall2, zg::components::entities::BodyType::Static});
        wall2RigidBody->setMass(2);
        wall2->addComponent(wall2RigidBody);
        wall2->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
            *wall2,
            std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(1, 5, 20) / 2.f),
            zg::components::entities::PhysicsMaterial{},
            glm::vec3(0),
            glm::quat(1, 0, 0, 0),
            false
        }));
        addEntity(wall2);
        wall3 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 43, 40), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec3(20, 5, 1), commonShaderConstants);
        wall3RigidBody = std::make_shared<zg::components::entities::RigidBody>(zg::components::entities::RigidBodyInfo{*wall3, zg::components::entities::BodyType::Static});
        wall3RigidBody->setMass(2);
        wall3->addComponent(wall3RigidBody);
        wall3->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
            *wall3,
            std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(20, 5, 1) / 2.f),
            zg::components::entities::PhysicsMaterial{},
            glm::vec3(0),
            glm::quat(1, 0, 0, 0),
            false
        }));
        addEntity(wall3);
        wall4 = std::make_shared<zg::entities::Cube>(window, *this, glm::vec3(50, 43, 60), glm::quat(1, 0, 0, 0), glm::vec3(1), glm::vec3(20, 5, 1), commonShaderConstants);
        wall4RigidBody = std::make_shared<zg::components::entities::RigidBody>(zg::components::entities::RigidBodyInfo{*wall4, zg::components::entities::BodyType::Static});
        wall4RigidBody->setMass(2);
        wall4->addComponent(wall4RigidBody);
        wall4->addComponent(std::make_shared<zg::components::entities::Collider>(zg::components::entities::ColliderInfo{
            *wall4,
            std::make_shared<zg::components::entities::BoxShapeData>(glm::vec3(20, 5, 1) / 2.f),
            zg::components::entities::PhysicsMaterial{},
            glm::vec3(0),
            glm::quat(1, 0, 0, 0),
            false
        }));
        addEntity(wall4);
        // cube controls
        f = KEYCODE_UP;
        b = KEYCODE_DOWN;
        l = KEYCODE_LEFT;
        r = KEYCODE_RIGHT;
        s = 32;
        std::function<void()> onFrontTickFunction = [&]()
        {
            cubeRigidBody->applyLocalForceToCenter({0, 0, 30});
        };
        std::function<void()> onBackTickFunction = [&]()
        {
            cubeRigidBody->applyLocalForceToCenter({0, 0, -30});
        };
        std::function<void()> onLeftTickFunction = [&]()
        {
            cubeRigidBody->applyLocalForceToCenter({30, 0, 0});
        };
        std::function<void()> onRightTickFunction = [&]()
        {
            cubeRigidBody->applyLocalForceToCenter({-30, 0, 0});
        };
        std::function<void()> onSpaceTickFunction = [&]()
        {
            zg::physics::CollisionMannifold* mannifoldPointer = 0;
            // if (cubeRigidBody->isTouching(*floorRigidBody, mannifoldPointer))
            // {
                cubeRigidBody->applyLocalForceToCenter({0, 518, 0});
            // }
        };
        //
        fID = window.addKeyUpdateHandler(f, onFrontTickFunction);
        bID = window.addKeyUpdateHandler(b, onBackTickFunction);
        lID = window.addKeyUpdateHandler(l, onLeftTickFunction);
        rID = window.addKeyUpdateHandler(r, onRightTickFunction);
        sID = window.addKeyUpdateHandler(s, onSpaceTickFunction);
        //
        thirdPerson = std::make_shared<zg::components::scenes::EntityThirdPersonCamera>(*this, *cube);
        addComponent(thirdPerson);
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