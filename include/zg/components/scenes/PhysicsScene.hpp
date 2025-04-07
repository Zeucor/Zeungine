#pragma once
#include <zg/interfaces/ISceneComponent.hpp>
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
		void resolveCollisions();
		// Updates the entity transforms based on simulation results
		void synchronizeTransforms();
	};
} // namespace zg::components::scenes
