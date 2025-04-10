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
namespace std
{
	template <>
	struct hash<std::pair<zg::components::entities::Collider*, zg::components::entities::Collider*>>
	{
		size_t operator()(const std::pair<zg::components::entities::Collider*, zg::components::entities::Collider*> &colliderPair) const
		{
			return ((std::uintptr_t)colliderPair.first) * ((std::uintptr_t)colliderPair.second);
		}
	};
}
namespace zg::components::scenes
{
	struct IGravity;
	const long double CCD_EPSILON = 1e-9L;
	const float SAT_EPSILON = 1e-6f; // Epsilon for SAT overlap checks
	struct TOIResult
	{
		long double toi = std::numeric_limits<long double>::infinity(); // Time of impact
		bool colliding = false; // Will they collide within the interval?
		physics::CollisionMannifold manifold; // Manifold computed *at* the predicted TOI (normal, points, depth=0)
	};
	struct PhysicsScene : interfaces::ISceneComponent
	{
	public:
		bool usingCCD = true;
		Scene& scene;
		IGravity* gravity = 0;
		std::vector<entities::RigidBody*> rigidBodies;
		std::vector<std::pair<entities::Collider*, entities::Collider*>> potentialPairs;
		std::vector<physics::CollisionMannifold> collisionContacts;
		long double& deltaTime;
		long double fixedTimeStep = 1.0 / 30.0;
		int totalSubSteps = 1;
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
		// Performs one CCD step of the simulation
		void stepSimulationCCD(long double totalDt);
		// Broadphase Helper using Swept AABBs
		void findPotentialCollisionPairs(long double dt);
		// Integrates position (updates based on force and torque)
		void updateTransforms(long double dt);
		// Integrates accelleration (updates based on velocity)
		void updateVelocities(long double dt);
		// Placeholder for collision detection
		void detectCollisions();
		// Placeholder for collision resolution
		void resolveCollisionImpulses(double dt);
		// Returns true if any correction was applied, false otherwise.
		void applyPositionalCorrection();
		// Updates the entity transforms based on simulation results
		void synchronizeTransforms();
		// Performs Separating Axis Theorem check for two box colliders
		bool performSATBoxBox(entities::Collider* boxA, entities::Collider* boxB, physics::CollisionMannifold& manifold);
		// Performs Separating Axis Theorem check for box-triangle
		bool performSATBoxTriangle(entities::Collider* boxCollider, const glm::vec3& triV0, const glm::vec3& triV1,
															 const glm::vec3& triV2, entities::Collider* meshCollider,
															 physics::CollisionMannifold& manifold);
		// Helper to project box vertices onto an axis
		void projectBoxOntoAxis(entities::Collider* boxCollider, const glm::vec3& axis, float& minProj, float& maxProj);
		// Helper to project triangle vertices onto an axis
		void projectTriangleOntoAxis(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& axis,
																 float& minProj, float& maxProj);
		// Helper to get world space axes of a box
		std::vector<glm::vec3> getBoxWorldAxes(entities::Collider* boxCollider);

		//
		std::vector<glm::vec3> getBoxWorldAxesInternal(entities::Collider* boxCollider);
		void projectBoxOntoAxisInternal(entities::Collider* boxCollider, const glm::vec3& axis, float& minProj,
																		float& maxProj);
		void projectTriangleOntoAxisInternal(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
																				 const glm::vec3& axis, float& minProj, float& maxProj);
		std::vector<glm::vec3> getBoxFaceVerticesInternal(entities::Collider* boxCollider,
																											const glm::vec3& faceNormalWorld);
		glm::vec3 getBoxFaceCenterInternal(entities::Collider* boxCollider, const glm::vec3& faceNormalWorld);
		std::vector<glm::vec3> getBestBoxFaceInternal(entities::Collider* boxCollider, const glm::vec3& directionWorld);
		std::vector<glm::vec3> clipPolygonAgainstPlaneInternal(const std::vector<glm::vec3>& polygonVertices,
																													 const glm::vec3& planeNormal, float planeDistance);
		void projectInterval(const glm::vec3* vertices, int numVertices, const glm::mat4& transform, const glm::vec3& axis,
												 float& minProj, float& maxProj);
		void projectBox(const glm::vec3& center, const glm::vec3& halfExtents, const std::vector<glm::vec3>& worldAxes,
										const glm::vec3& axis, float& minProj, float& maxProj);
		void projectTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& axis,
												 float& minProj, float& maxProj);
		/**
		 * @brief tests an axis using swept algorithm
		 * minA, maxA, minB, maxB: Projections at t=0
		 * projectedRelVel: Dot product of relative velocity and axis
		 * tEnter: Output: Time when overlap starts along this axis
		 * tExit: Output: Time when overlap ends along this axis
		 **/
		bool testAxisSwept(float minA, float maxA, float minB, float maxB, float projectedRelVel, long double dt,
											 long double& tEnter, long double& tExit);
		TOIResult findTOIBoxBox(zg::components::entities::Collider* boxColliderA,
														zg::components::entities::Collider* boxColliderB, long double dt);
		TOIResult findTOIBoxTriangle(zg::components::entities::Collider* boxCollider, const glm::vec3& triV0,
																 const glm::vec3& triV1, const glm::vec3& triV2, const glm::vec3& triVel0,
																 const glm::vec3& triVel1, const glm::vec3& triVel2,
																 zg::components::entities::Collider* meshCollider, long double dt);
	};
} // namespace zg::components::scenes
