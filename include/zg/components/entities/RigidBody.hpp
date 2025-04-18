#pragma once
#include <zg/glm.hpp>
#include <zg/interfaces/IEntityComponent.hpp>
#include <zg/physics/CollisionManifold.hpp>
#include <zg/physics/ToJolt.hpp>
namespace zg
{
	struct Entity;
}
namespace zg::components::scenes
{
	struct PhysicsScene;
}
namespace zg::components::entities
{
	enum class BodyType
	{
		Static, // Infinite mass, doesn't move, unaffected by forces (e.g., ground)
		Kinematic, // Infinite mass, moved via code (setPosition/setRotation), not forces (e.g., moving platforms)
		Dynamic // Finite mass, moved by forces and collisions
	};
	struct RigidBodyInfo
	{
		Entity& entity;
		BodyType bodyType;
		float mass = 1.0f;
		float linearDamping = 0.8f;
		float angularDamping = 0.8f;
		bool useGravity = true;
		bool isKinematicInitially = false;
		glm::vec<3, bool> freezeRotationAxes = {false, false, false};
		glm::vec<3, bool> freezeVelocityAxes = {false, false, false};
	};
	struct Collider;
	struct RigidBody : interfaces::IEntityComponent
	{
		friend Collider;
		friend scenes::PhysicsScene;

	protected:
		RigidBodyInfo info;
		glm::vec3* position = 0;
		glm::quat* rotation = 0;
		scenes::PhysicsScene* physicsScene = nullptr; // Pointer to the scene managing physics
		JPH::BodyInterface* joltBodyInterface = nullptr; // Cached pointer to Jolt's body interface
		JPH::Body* body = 0;
		std::vector<Collider*> colliders; // Colliders attached to this body
		JPH::BodyID joltBodyID;
		bool sleeping = false;
		std::unordered_map<RigidBody*, physics::CollisionManifold> activeRigidBodyManifolds;
		std::mutex mutex;

	public:
		RigidBody(const RigidBodyInfo& info);

	protected:
		void addCollider(Collider* collider);
		void removeCollider(Collider* collider);
		void recreateJoltBody();

	public:
		void onAttached() override;
		void onUpdate() override;
		void onDetached() override;
		void applyForce(glm::vec3 worldForce, glm::vec3 worldPoint); // Apply force at world point with deltaTime
		void applyLocalForceToCenter(glm::vec3 localForce);
		void applyForceToCenter(glm::vec3 force); // Apply force at center of mass
		void applyTorque(glm::vec3 torque); // Apply rotational force
		void clearForces(); // Clears force and torque accumulators (called by Physics system each step)
		const glm::vec3& getPosition() const;
		void setPosition(glm::vec3 newPosition);
		const glm::quat& getOrientation() const;
		void setOrientation(glm::quat newOrientation);
		const glm::vec3 getLinearVelocity() const;
		RigidBodyInfo& getInfo();
		const RigidBodyInfo& getInfo() const;
		void setLinearVelocityX(float va);
		void setLinearVelocityY(float va);
		void setLinearVelocityZ(float va);
		void setAngularVelocityX(float va);
		void setAngularVelocityY(float va);
		void setAngularVelocityZ(float va);
		void setLinearVelocity(glm::vec3 newLinearVelocity);
		void setAngularVelocity(glm::vec3 newAngularVelocity);
		bool isSleeping();
		void setSleeping(bool newSleeping);
		JPH::BodyID getJoltBodyID() const;

		glm::vec3 getAngularVelocity() const;
		float getMass() const;
		void setMass(float newMass);
		float getInverseMass() const;
		const std::vector<Collider*>& getColliders() const;
		bool getUseGravity() const;
		glm::vec3 getFreezeRotationAxes() const;
		glm::vec3 getFreezeVelocityAxes() const;
		glm::vec3 getForceAccumulator() const;
		glm::vec3 getTorqueAccumulator() const;
		float getLinearDamping();
		float getAngularDamping();
		void translate(glm::vec3 deltaPos);
		bool isStatic() const;
		bool isKinematic() const;
		bool isDynamic() const;
		glm::vec3 getCenterAtTime(float t) const;
		void update(float dt, bool clearForces = true);
		bool isTouching(RigidBody& rigidBody, physics::CollisionManifold*& ManifoldPointer);
		void addActiveManifold(const physics::CollisionManifold& Manifold);
		void removeActiveManifold(RigidBody& otherRb);
		void clearActiveManifolds();
	};
} // namespace zg::components::entities
