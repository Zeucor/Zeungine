#pragma once
#include "SceneComponent.hpp"
#include <zg/physics/CollisionManifold.hpp>
#include <zg/conversions/ToJolt.hpp>
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
	using JoltIDComponentPair = std::pair<JPH::BodyID, components::entities::EntityComponent*>;
	components::scenes::SceneComponentCreateInfo PhysicsSceneFactory();
	class ZGContactListener : public JPH::ContactListener
	{
	public:
		ZGContactListener(const std::vector<size_t*>& sceneIndexStack, size_t physicsSceneID);
		void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold,
												JPH::ContactSettings& ioSettings) override;
		void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold,
														JPH::ContactSettings& ioSettings) override;
		void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

	private:
		std::vector<size_t*> sceneIndexStack;
		size_t physicsSceneID;
		physics::CollisionManifold constructManifold(components::entities::EntityComponent* ec1, components::entities::EntityComponent* ec2, const JPH::ContactManifold& inManifold);
	};
} // namespace zg::components::scenes
