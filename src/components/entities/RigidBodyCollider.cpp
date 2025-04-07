#include <zg/components/entities/RigidBodyCollider.hpp>
using namespace zg::components::entities;
RigidBodyCollider::RigidBodyCollider(Entity& entity, bool isStatic):
    entity(entity),
    isStatic(isStatic)
{}
void RigidBodyCollider::onAttached()
{
}
void RigidBodyCollider::onUpdate()
{
}
void RigidBodyCollider::onDetached()
{
}