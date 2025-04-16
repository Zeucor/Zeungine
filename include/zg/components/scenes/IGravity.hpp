#pragma once
#include <zg/interfaces/ISceneComponent.hpp>
namespace zg::components::scenes
{
    struct PhysicsScene;
    struct IGravity : interfaces::ISceneComponent
    {
        IGravity();
        virtual ~IGravity() = default;
        virtual void onAttached() = 0;
		virtual void onUpdate() = 0;
        virtual void onDetached() = 0;
        virtual void applyGravity(PhysicsScene& physicsScene, float dt) = 0;
    };
}