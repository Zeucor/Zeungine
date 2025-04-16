#pragma once
#include <zg/glm.hpp>
#include <zg/interfaces/IEntityComponent.hpp>
#include <zg/physics/CollisionMannifold.hpp>
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

	private:
		RigidBodyInfo info;
		glm::mat4* transform = 0;
		glm::vec3* position = 0;
		glm::quat* rotation = 0;
		scenes::PhysicsScene* physicsScene = 0;
		std::vector<Collider*> colliders;
		glm::vec3 linearVelocity = {0.0f, 0.0f, 0.0f};
		glm::vec3 angularVelocity = {0.0f, 0.0f, 0.0f};
		glm::vec3 linearAcceleration = {0.0f, 0.0f, 0.0f};
		glm::vec3 angularAcceleration = {0.0f, 0.0f, 0.0f};
		glm::vec3 forceAccumulator = {0.0f, 0.0f, 0.0f};
		glm::vec3 torqueAccumulator = {0.0f, 0.0f, 0.0f};
		glm::mat3 inverseInertiaTensorBody = glm::mat3(0.f);
		bool sleeping = false;
		std::unordered_map<RigidBody*, physics::CollisionMannifold> activeRigidBodyMannifolds;

	public:
		RigidBody(const RigidBodyInfo& info);

	protected:
		void addCollider(Collider* collider);
		void removeCollider(Collider* collider);

	public:
		void onAttached() override;
		void onUpdate() override;
		void onDetached() override;
		void applyForce(glm::vec3 force, glm::vec3 worldPoint, float dt); // Apply force at world point with deltaTime
		void applyLocalForceToCenter(glm::vec3 localForce, float dt);
		void applyForceToCenter(glm::vec3 force, float dt); // Apply force at center of mass
		void applyTorque(glm::vec3 torque, float dt); // Apply rotational force
		void clearForces(); // Clears force and torque accumulators (called by Physics system each step)
		const glm::vec3& getPosition() const;
		const glm::quat& getOrientation() const;
		const glm::mat3& getInverseInertiaTensorBody() const { return inverseInertiaTensorBody; }
		const glm::vec3 getLinearVelocity() const { return linearVelocity; }
		const glm::vec3 getLinearAcceleration() const { return linearAcceleration; }
		RigidBodyInfo& getInfo() { return info; }
		const RigidBodyInfo& getInfo() const { return info; }
		void setLinearVelocityX(float va)
		{
			linearVelocity.x = va;
			// calculate forces linearVelocity.x
		}
		void setLinearVelocityY(float va)
		{
			linearVelocity.y = va;
			// calculate forces linearVelocity.y
		}
		void setLinearVelocityZ(float va)
		{
			linearVelocity.z = va;
			// calculate forces linearVelocity.z
		}
		void setAngularVelocityX(float va)
		{
			angularVelocity.x = va;
			// calculate forces angularVelocity.x
		}
		void setAngularVelocityY(float va)
		{
			angularVelocity.y = va;
			// calculate forces angularVelocity.y
		}
		void setAngularVelocityZ(float va)
		{
			angularVelocity.z = va;
			// calculate forces angularVelocity.z
		}
		void setLinearVelocity(glm::vec3 newLinearVelocity)
		{
			linearVelocity = newLinearVelocity;
		}
		void setAngularVelocity(glm::vec3 newAngularVelocity)
		{
			angularVelocity = newAngularVelocity;
		}
		bool getSleeping()
		{
			return sleeping;
		}
		void setSleeping(bool newSleeping)
		{
			if (sleeping != newSleeping)
				sleeping = newSleeping;
		}

		glm::vec3 getAngularVelocity() const { return angularVelocity; }
		float getMass() const { return info.mass; }
		void setMass(float newMass) { info.mass = newMass; }
		float getInverseMass() const
		{
			return (info.bodyType == BodyType::Dynamic && info.mass > 0.0f) ? 1.0f / info.mass : 0.0f;
		}
		const glm::mat4& getTransform() const { return *transform; }
		const std::vector<Collider*>& getColliders() const { return colliders; }
		bool getUseGravity() const { return info.useGravity; }
		glm::vec3 getFreezeRotationAxes() const { return info.freezeRotationAxes; }
		glm::vec3 getFreezeVelocityAxes() const { return info.freezeVelocityAxes; }
		glm::vec3 getForceAccumulator() const { return forceAccumulator; }
		glm::vec3 getTorqueAccumulator() const { return torqueAccumulator; }
		float getLinearDamping() { return info.linearDamping; }
		float getAngularDamping() { return info.angularDamping; }
		// void setPosition(glm::vec3 pos);
		// void setRotation(glm::vec3 rot);
		void translate(glm::vec3 deltaPos);
		bool isStatic() const;
		bool isKinematic() const;
		bool isDynamic() const;
		glm::mat3 getInverseInertiaTensorWorld() const;
		glm::vec3 getCenterAtTime(float t) const { return *position + linearVelocity * t; }
		void update(float dt, bool clearForces = true);
		bool isTouching(RigidBody& rigidBody, physics::CollisionMannifold*& mannifoldPointer);
		void addActiveMannifold(const physics::CollisionMannifold& mannifold);
		void clearActiveMannifolds();
	};
} // namespace zg::components::entities
