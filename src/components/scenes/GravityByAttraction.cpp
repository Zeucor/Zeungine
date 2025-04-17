#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/GravityByAttraction.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
#include <zg/Scene.hpp>
using namespace zg::components::scenes;
GravityByAttraction::GravityByAttraction(Scene& scene, float gravitationalConstant) :
		scene(scene), gravitationalConstant(gravitationalConstant)
{
}
void GravityByAttraction::onAttached()
{
	auto physicsScene = std::dynamic_pointer_cast<PhysicsScene>(scene.getComponentByName("PhysicsScene"));
	if (physicsScene)
		physicsScene->setGravity(this);
}
void GravityByAttraction::onUpdate() {}
void GravityByAttraction::onDetached() {}
void GravityByAttraction::applyGravity(PhysicsScene& physicsScene, float dt)
{
	auto& rigidBodies = physicsScene.rigidBodiesJoltID;
	// Accumulate forces on each body
	for (auto& rbPairA : rigidBodies)
	{
		entities::RigidBody* rigidBodyA = rbPairA.first;
		if (!rigidBodyA || !rigidBodyA->isDynamic() || !rigidBodyA->getUseGravity())
		{
			continue; // Skip if null, not dynamic, or doesn't use gravity
		}

		// Get properties of body A
		float bodyAMass = rigidBodyA->getMass(); // Use info.mass directly
		if (std::abs(bodyAMass) < 1e-6f)
			continue; // Skip bodies with zero mass

		// Extract position correctly from the const mat4*
		glm::vec3 totalForceOnA = glm::vec3(0.0f); // Accumulate force here
		const auto& collidersA = rigidBodyA->getColliders();
		for (auto& colliderA : collidersA)
		{
			auto AABB_A = colliderA->getWorldAABB();

			for (auto& rbPairB : rigidBodies) // Corrected loop bound: j < rigidBodiesSize
			{
				entities::RigidBody* rigidBodyB = rbPairB.first;
				if (rigidBodyA == rigidBodyB)
					continue; // Skip self

				if (!rigidBodyB)
					continue; // Skip null pointers

				const auto& collidersB = rigidBodyB->getColliders();

				for (auto& colliderB : collidersB)
				{
					auto AABB_B = colliderB->getWorldAABB();

					// Get properties of body B
					float bodyBMass = rigidBodyB->getMass();
					if (std::abs(bodyBMass) < 1e-6f)
						continue; // Skip bodies with zero mass

					// Extract position correctly
					glm::vec3 n;
					float pd, md;
					int axis;
					auto overlaps = AABB_A.overlaps(AABB_B, n, pd, md, axis);

					// Calculate force between A and B
					auto halfExtentsA = colliderA->getColliderInfo().shapeData->getHalfExtents();
					auto halfExtentsB = colliderB->getColliderInfo().shapeData->getHalfExtents();
					md += halfExtentsA[axis] + halfExtentsB[axis];

					glm::vec3 directionAB = n; // Normalize

					// Calculate gravitational force magnitude: F = G * (m1 * m2) / r^2
					float forceMagnitude = gravitationalConstant * (bodyAMass * bodyBMass) / (md * md);

					// Calculate force vector applied ON A BY B
					glm::vec3 forceOnA = directionAB * forceMagnitude;

					// Accumulate the force on body A
					totalForceOnA += forceOnA;
				}
			}
		}
		// Apply the total accumulated force to body A's center of mass
		rigidBodyA->applyForceToCenter(totalForceOnA);
	}
}
