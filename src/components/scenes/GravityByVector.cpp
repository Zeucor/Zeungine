#include <zg/components/scenes/GravityByVector.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
#include <zg/components/entities/RigidBody.hpp>
using namespace zg::components::scenes;
GravityByVector::GravityByVector(glm::vec3 gravity):
    gravity(gravity)
{}
void GravityByVector::onAttached()
{
}
void GravityByVector::onUpdate()
{
}
void GravityByVector::onDetached()
{
}
void GravityByVector::applyGravity(PhysicsScene& physicsScene)
{
    for (auto rigidBody : physicsScene.rigidBodies) // Use range-based for loop
    {
        // Apply gravity only to dynamic bodies that have useGravity enabled
        if (rigidBody && rigidBody->isDynamic() && rigidBody->info.useGravity)
        {
            // Apply force F = m * g
            rigidBody->applyForceToCenter(gravity * rigidBody->info.mass);
        }
    }
}