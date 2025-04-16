#pragma once
#include "IGravity.hpp"
namespace zg::components::scenes
{
    struct GravityByAttraction : IGravity
    {
        float gravitationalConstant;
        GravityByAttraction(float gravitationalConstant);
        void onAttached() override;
		void onUpdate() override;
        void onDetached() override;
        void applyGravity(PhysicsScene& physicsScene, float dt) override;
    };
}