#pragma once
#include "IGravity.hpp"
namespace zg::components::scenes
{
    struct GravityByVector : IGravity
    {
        Scene& scene;
        glm::vec3 gravity;
        GravityByVector(Scene& scene, glm::vec3 gravity);
        void onAttached() override;
		void onUpdate() override;
        void onDetached() override;
        void applyGravity(PhysicsScene& physicsScene, float dt) override;
    };
}