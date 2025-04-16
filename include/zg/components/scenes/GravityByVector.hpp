#pragma once
#include "IGravity.hpp"
namespace zg::components::scenes
{
    struct GravityByVector : IGravity
    {
        glm::vec3 gravity;
        GravityByVector(glm::vec3 gravity);
        void onAttached() override;
		void onUpdate() override;
        void onDetached() override;
        void applyGravity(PhysicsScene& physicsScene, float dt) override;
    };
}