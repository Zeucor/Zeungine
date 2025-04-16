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
		size_t operator()(
			const std::pair<zg::components::entities::Collider*, zg::components::entities::Collider*>& colliderPair) const
		{
			return ((std::uintptr_t)colliderPair.first) * ((std::uintptr_t)colliderPair.second);
		}
	};
} // namespace std
namespace zg::components::scenes
{
	struct Constraint
	{
		enum Normal
		{
			AtOrAbove, // Self's relevant boundary should be >= Other's relevant boundary
			AtOrBelow // Self's relevant boundary should be <= Other's relevant boundary
		};

		Normal normal;
		float referencePosition;
		float& positionSelf;
		Constraint(Normal normal, float referencePosition, float& positionSelf) :
				normal(normal), referencePosition(referencePosition),
				positionSelf(positionSelf)
		{
		}

		// Applies the constraint, adjusting positionSelf based on rotation
		void apply()
		{
			if (normal == AtOrAbove)
			{
				if (positionSelf < referencePosition)
				{
					positionSelf = referencePosition;
				}
			}
			else
			{
				if (positionSelf > referencePosition)
				{
					positionSelf = referencePosition;
				}
			}
		}
	};
	struct IGravity;
	struct TOIResult
	{
		float toi = std::numeric_limits<float>::infinity(); // Time of impact
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
		std::vector<physics::CollisionMannifold> collisionContacts;
		std::unordered_map<entities::RigidBody*, std::vector<Constraint>> colliderConstraints;
		long double& deltaTime;
		float fixedTimeStep = 1.0 / 30.0;
		int totalSubSteps = 1;
		float timeAccumulator = 0.0f;
		PhysicsScene(Scene& scene);
		void onAttached() override;
		void onUpdate() override;
		void onDetached() override;
		void registerRigidBody(entities::RigidBody* rigidBody);
		void unregisterRigidBody(entities::RigidBody* rigidBody);
		static std::pair<glm::vec3, glm::quat> getTransformsAtTime(entities::RigidBody* rb, float t);
		void updateVelocities(entities::RigidBody* rb, float t);

	private:
		// Performs one fixed step of the simulation
		// void stepSimulation(float dt);
		// Performs one CCD step of the simulation
		void stepSimulationCCD(float totalDt);
		// Broadphase Helper using Swept AABBs
		void findPotentialCollisionPairs(float dt,
																		 std::vector<std::pair<entities::Collider*, entities::Collider*>>& potentialPairs);
		// Broadphase Helper using Swept AABBs by RigidBody
		void findPotentialCollisionPairs(entities::RigidBody& rigidBody, float dt,
																		 std::vector<std::pair<entities::Collider*, entities::Collider*>>& potentialPairs);
		// Integrates position (updates based on force and torque)
		void updateTransforms(float dt);
		// Integrates accelleration (updates based on velocity)
		void updateVelocities(float dt);
		// Placeholder for collision detection
		// void detectCollisions();
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
		// void projectBoxOntoAxis(entities::Collider* boxCollider, const glm::vec3& axis, float& minProj, float& maxProj);
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
		TOIResult findTOIBoxBox(zg::components::entities::Collider* boxColliderA,
														zg::components::entities::Collider* boxColliderB, float dt);
		TOIResult findTOIBoxTriangle(zg::components::entities::Collider* boxCollider, const glm::vec3& triV0,
																 const glm::vec3& triV1, const glm::vec3& triV2, const glm::vec3& triVel0,
																 const glm::vec3& triVel1, const glm::vec3& triVel2,
																 zg::components::entities::Collider* meshCollider, float dt);
		void applyConstraints(entities::RigidBody* body);
		void resolveConstraint(const physics::CollisionMannifold& collisionMannifold);
		void applyContstraints();
		void clearConstraints();
	};
} // namespace zg::components::scenes
