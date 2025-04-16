#pragma once
#include <zg/interfaces/ISceneComponent.hpp>
#include <zg/physics/CollisionManifold.hpp>
#include <zg/physics/ToJolt.hpp>
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
	struct PhysicsScene : interfaces::ISceneComponent
	{
	public:
		Scene& scene;
		IGravity* gravity = 0;
		std::unordered_map<entities::RigidBody*, JPH::BodyID> rigidBodiesJoltID;
		std::unordered_map<JPH::BodyID, entities::RigidBody*> joltIDRigidBodies;
		std::vector<physics::CollisionManifold> collisionContacts;
		std::unique_ptr<ZGContactListener> contactListener;
		long double& deltaTime;
		PhysicsScene(Scene& scene);
		void onAttached() override;
		void onUpdate() override;
		void onDetached() override;
		void registerRigidBody(entities::RigidBody* rigidBody);
		void unregisterRigidBody(entities::RigidBody* rigidBody);
		JPH::PhysicsSystem& GetJoltPhysicsSystem();
		JPH::BodyInterface& GetBodyInterface();
		const JPH::BodyLockInterface& GetBodyLockInterface();

	private:
		void stepSimulation(float totalDt);
		void synchronize();
		// --- Jolt Core Systems ---
		std::unique_ptr<JPH::Factory> mFactory;
		std::unique_ptr<JPH::TempAllocatorImpl> mTempAllocator;
		std::unique_ptr<JPH::JobSystemThreadPool> mJobSystem;

		// --- Jolt Physics System ---
		std::unique_ptr<JPH::PhysicsSystem> mPhysicsSystem;

		// --- Jolt Layer/Collision Filtering ---
		BPLayerInterfaceImpl mBroadPhaseLayerInterface;
		// Note: Using function pointers directly here. You might wrap these in classes.
		ZGObjectVsBroadPhaseLayerFilter mObjectVsBroadPhaseLayerFilter;
		ZGObjectLayerPairFilter mObjectLayerPairFilter;
	};

	class ZGContactListener : public JPH::ContactListener
	{
	public:
		ZGContactListener(PhysicsScene& physicsScene);
		void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold,
												JPH::ContactSettings& ioSettings) override;
		void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold,
														JPH::ContactSettings& ioSettings) override;
		void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

	private:
		PhysicsScene& physicsScene;
		std::mutex mutex;
		physics::CollisionManifold constructManifold(entities::RigidBody* rb1, entities::RigidBody* rb2, const JPH::ContactManifold& inManifold);
	};
} // namespace zg::components::scenes
