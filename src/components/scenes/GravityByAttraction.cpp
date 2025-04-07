#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/GravityByAttraction.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
using namespace zg::components::scenes;
GravityByAttraction::GravityByAttraction(float gravitationalConstant) : gravitationalConstant(gravitationalConstant) {}
void GravityByAttraction::onAttached() {}
void GravityByAttraction::onUpdate() {}
void GravityByAttraction::onDetached() {}
void GravityByAttraction::applyGravity(PhysicsScene& physicsScene)
{
	auto& rigidBodies = physicsScene.rigidBodies;
	size_t rigidBodiesSize = rigidBodies.size(); // Use size_t for size
	auto rigidBodiesData = rigidBodies.data(); // Using data() is fine but direct indexing works too

	// Accumulate forces on each body
	for (size_t i = 0; i < rigidBodiesSize; ++i)
	{
		entities::RigidBody* rigidBodyA = rigidBodiesData[i];
        if (!rigidBodyA || !rigidBodyA->isDynamic() || !rigidBodyA->info.useGravity) {
             continue; // Skip if null, not dynamic, or doesn't use gravity
        }

        // Get properties of body A
		float bodyAMass = rigidBodyA->info.mass; // Use info.mass directly
        if (std::abs(bodyAMass) < 1e-6f) continue; // Skip bodies with zero mass

        // Extract position correctly from the const mat4*
		glm::vec3 positionA = glm::vec3((*rigidBodyA->transform)[3]);

        glm::vec3 totalForceOnA = glm::vec3(0.0f); // Accumulate force here

		for (size_t j = 0; j < rigidBodiesSize; ++j) // Corrected loop bound: j < rigidBodiesSize
		{
			if (i == j) continue; // Skip self

            entities::RigidBody* rigidBodyB = rigidBodiesData[j];
             if (!rigidBodyB) continue; // Skip null pointers

            // Get properties of body B
			float bodyBMass = rigidBodyB->info.mass;
            if (std::abs(bodyBMass) < 1e-6f) continue; // Skip bodies with zero mass

            // Extract position correctly
			glm::vec3 positionB = glm::vec3((*rigidBodyB->transform)[3]);

            // Calculate force between A and B
			glm::vec3 vectorAB = positionB - positionA;
			float distanceSq = glm::dot(vectorAB, vectorAB); // Use distance squared to avoid sqrt

            // Avoid division by zero or extremely large forces at close range
            const float minDistanceSq = 1e-4f; // Minimum distance squared threshold
            if (distanceSq < minDistanceSq) {
                continue; // Skip calculation if bodies are too close
            }

            float distance = std::sqrt(distanceSq); // Calculate distance only when needed
			glm::vec3 directionAB = vectorAB / distance; // Normalize

            // Calculate gravitational force magnitude: F = G * (m1 * m2) / r^2
			float forceMagnitude = gravitationalConstant * (bodyAMass * bodyBMass) / distanceSq;

            // Calculate force vector applied ON A BY B
			glm::vec3 forceOnA = directionAB * forceMagnitude;

            // Accumulate the force on body A
            totalForceOnA += forceOnA;
		}
        // Apply the total accumulated force to body A's center of mass
		rigidBodyA->applyForceToCenter(totalForceOnA);
	}
}
