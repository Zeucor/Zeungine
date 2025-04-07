#pragma once
#include <zg/interfaces/ISceneComponent.hpp>
#include <zg/physics/CollisionMannifold.hpp>
namespace zg
{
	struct Scene;
}
namespace zg::components::entities
{
	struct RigidBody;
}
namespace zg::components::scenes
{
	struct IGravity;
	struct PhysicsScene : interfaces::ISceneComponent
	{
        public:
		Scene& scene;
		IGravity* gravity = 0;
		std::vector<entities::RigidBody*> rigidBodies;
        std::vector<std::pair<entities::Collider*, entities::Collider*>> potentialPairs;
        std::vector<physics::CollisionManifold> collisionContacts;
		long double& deltaTime;
		long double fixedTimeStep = 1.0 / 30.0;
        int maxSubSteps = 10;
        long double timeAccumulator = 0.0f;
		PhysicsScene(Scene& scene);
		void onAttached() override;
		void onUpdate() override;
		void onDetached() override;
		void registerRigidBody(entities::RigidBody* rigidBody);
		void unregisterRigidBody(entities::RigidBody* rigidBody);

	private:
		// Performs one fixed step of the simulation
		void stepSimulation(long double dt);
		// Integrates motion (updates velocity and position based on forces)
		void integrate(long double dt);
		// Placeholder for collision detection
		void detectCollisions();
		// Placeholder for collision resolution
		void resolveCollisions(long double dt);
		// Updates the entity transforms based on simulation results
		void synchronizeTransforms();
        // Performs Separating Axis Theorem check for two box colliders
        bool performSATBoxBox(entities::Collider* boxA, entities::Collider* boxB, physics::CollisionManifold& manifold);
        // Helper to project box vertices onto an axis
        void projectBoxOntoAxis(entities::Collider* boxCollider, const glm::vec3& axis, float& minProj, float& maxProj);
        // Helper to get world space axes of a box
        std::vector<glm::vec3> getBoxWorldAxes(entities::Collider* boxCollider);
	};
} // namespace zg::components::scenes
