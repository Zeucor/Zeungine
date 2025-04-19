#pragma once
#include "SceneComponent.hpp"
#include <zg/physics/CollisionManifold.hpp>
#include <zg/physics/ToJolt.hpp>
#include <zg/system/Budget.hpp>
#include <thread>
#include <mutex>
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
	struct ZGContactListener;
	struct GravityByVector;
	struct GravityByAttraction;
	using JoltIDComponentPair = std::pair<JPH::BodyID, interfaces::IComponent*>;
	components::scenes::SceneComponentCreateInfo PhysicsSceneFactory(long double deltaTime);
	class ZGContactListener : public JPH::ContactListener
	{
	public:
		ZGContactListener(SceneComponent& physicsScene);
		void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold,
												JPH::ContactSettings& ioSettings) override;
		void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold,
														JPH::ContactSettings& ioSettings) override;
		void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

	private:
		SceneComponent& physicsScene;
		physics::CollisionManifold constructManifold(interfaces::IComponent* ec1, interfaces::IComponent* ec2, const JPH::ContactManifold& inManifold);
	};
} // namespace zg::components::scenes
