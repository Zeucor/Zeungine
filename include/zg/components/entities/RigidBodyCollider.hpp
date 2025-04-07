#pragma once
#include <zg/interfaces/IEntityComponent.hpp>
namespace zg
{
    struct Entity;
}
namespace zg::components::entities
{
    struct RigidBodyCollider : interfaces::IEntityComponent
    {
        Entity& entity;
        bool isStatic;
        RigidBodyCollider(Entity& entity, bool isStatic);
		void onAttached() override;
		void onUpdate() override;
		void onDetached() override;
    };
}