#include <unordered_set>
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/IGravity.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
#include <zg/physics/OBB.hpp>
#include <zg/physics/Plane.hpp>
#include <zg/physics/Projection.hpp>
using namespace zg::components::scenes;
using zg::components::entities::BoxShapeData;
using zg::components::entities::CapsuleShapeData;
using zg::components::entities::Collider;
using zg::components::entities::ConvexHullShapeData;
using zg::components::entities::MeshShapeData;
using zg::components::entities::RigidBody;
using zg::components::entities::ShapeType;
using zg::components::entities::SphereShapeData;
using zg::physics::AABB;
using zg::physics::CollisionMannifold;
using zg::physics::OBB;
using zg::physics::Plane;
using zg::physics::Projection;
PhysicsScene::PhysicsScene(Scene& scene) :
		ISceneComponent("PhysicsScene"), scene(scene), deltaTime(scene.window.deltaTime)
{
	std::cout << "PhysicsScene created." << std::endl;
}
void PhysicsScene::onAttached()
{
	gravity = dynamic_cast<IGravity*>(scene.getComponentByName("IGravity").get());
	std::cout << "PhysicsScene attached." << std::endl;

	std::cout << "Initializing Jolt Physics..." << std::endl;

	// --- Jolt Allocation Hooks (Optional but recommended) ---
	// Register allocation hook if desired (e.g., for memory tracking)
	JPH::RegisterDefaultAllocator();

	// --- Jolt Factory ---
	JPH::Factory::sInstance = new JPH::Factory();

	// --- Jolt Type Registration ---
	// Register all Jolt physics types
	JPH::RegisterTypes();

	// --- Jolt Allocators & Job System ---
	// We need a temp allocator for temporary allocations during the physics update.
	// We're pre-allocating 10 MB. Adjust this based on your needs.
	mTempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

	// We need a job system that will execute physics jobs on multiple threads.
	// MaxConcurrentJobs determines the maximum number of jobs that can run in parallel.
	// MaxJobs determines the maximum number of jobs that can be added to the queue.
	// Consider using JPH::JobSystemSingleThreaded for debugging or simpler scenarios.
	mJobSystem = std::make_unique<JPH::JobSystemThreadPool>(2048, 128, std::thread::hardware_concurrency() - 1);

	// --- Jolt Physics System Creation ---
	mPhysicsSystem = std::make_unique<JPH::PhysicsSystem>();
#define JOLT_MAX_BODIES 100000
#define JOLT_NUM_BODY_MUTEXES 10000
#define JOLT_MAX_BODY_PAIRS 1000
#define JOLT_MAX_CONTACT_CONSTRAINTS 1000
	mPhysicsSystem->Init(JOLT_MAX_BODIES, JOLT_NUM_BODY_MUTEXES, JOLT_MAX_BODY_PAIRS, JOLT_MAX_CONTACT_CONSTRAINTS,
											 mBroadPhaseLayerInterface, mObjectVsBroadPhaseLayerFilter, mObjectLayerPairFilter);

	// --- Jolt Listeners (Optional) ---
	// Initialize and set your contact/activation listeners here if you have them
	// Example:
	// mContactListener = new MyContactListener();
	// mPhysicsSystem->SetContactListener(mContactListener);
	// mPhysicsSystem->SetBodyActivationListener(mBodyActivationListener); // If you have one

	std::cout << "Jolt Physics Initialized Successfully." << std::endl;
}
void PhysicsScene::onUpdate() { stepSimulation(deltaTime); }
void PhysicsScene::onDetached()
{
	rigidBodiesJoltID.clear();
	joltIDRigidBodies.clear();
	JPH::UnregisterTypes();
	delete JPH::Factory::sInstance;
	JPH::Factory::sInstance = nullptr;
	gravity = nullptr; // Clear pointer, don't delete if owned by scene
	std::cout << "PhysicsScene detached." << std::endl;
}
void PhysicsScene::registerRigidBody(RigidBody* rigidBody)
{
	auto joltBodyID = rigidBody->getJoltBodyID();
	rigidBodiesJoltID[rigidBody] = joltBodyID;
	joltIDRigidBodies[joltBodyID] = rigidBody;
	std::cout << "Registered RigidBody." << std::endl;
}
void PhysicsScene::unregisterRigidBody(RigidBody* rigidBody)
{
	if (rigidBody)
	{
		auto joltBodyID = rigidBody->getJoltBodyID();
		rigidBodiesJoltID.erase(rigidBody);
		joltIDRigidBodies.erase(joltBodyID);
	}
}
JPH::PhysicsSystem& PhysicsScene::GetJoltPhysicsSystem() { return *mPhysicsSystem; }
JPH::BodyInterface& PhysicsScene::GetBodyInterface() { return mPhysicsSystem->GetBodyInterface(); }
const JPH::BodyLockInterface& PhysicsScene::GetBodyLockInterface()
{
	return mPhysicsSystem->GetBodyLockInterfaceNoLock();
}
void PhysicsScene::stepSimulation(float dt)
{
	mPhysicsSystem->Update(dt, 10, mTempAllocator.get(), mJobSystem.get());
	synchronize();
}
void PhysicsScene::synchronize()
{

	if (!mPhysicsSystem)
		return;

	auto& body_interface = mPhysicsSystem->GetBodyInterface();

	// Iterate through *active* Jolt bodies. Inactive bodies don't need updating.
	// Note: This requires locking if accessed from multiple threads.
	// Consider processing only bodies linked to your active entities if possible.
	const JPH::BodyLockInterface& lock_interface = GetBodyLockInterface(); // Use NoLock if on main thread
	// const JPH::BodyLockInterface& lock_interface = mPhysicsSystem->GetBodyLockInterface(); // Use locking version if
	// needed

	JPH::BodyIDVector active_body_ids;
	mPhysicsSystem->GetActiveBodies(JPH::EBodyType::RigidBody, active_body_ids); // Get IDs of currently active bodies

	for (const auto& bodyID : active_body_ids)
	{
		// Find the corresponding Zeungine entity
		auto entityIt = joltIDRigidBodies.find(bodyID);
		if (entityIt != joltIDRigidBodies.end())
		{
			auto rb = entityIt->second;

			// Lock the body to read its state safely (use ReadLock)
			JPH::BodyLockRead lock(lock_interface, bodyID);
			if (lock.Succeeded()) // Important: Check if lock was successful
			{
				const auto& body = lock.GetBody();

				// Only update transforms for non-static bodies
				if (!body.IsStatic())
				{
					// Get world position and rotation from Jolt
					auto position = body.GetPosition();
					auto rotation = body.GetRotation();

					// Update the Zeungine TransformComponent
					(*rb->position) = ToJolt<JPH::Vec3, glm::vec3>(position);
					(*rb->rotation) = ToJolt<JPH::Quat, glm::quat>(rotation);

					// Optional: Update linear/angular velocity in RigidBodyComponent if needed
					// RigidBodyComponent* rbComp = Zeungine::GetComponent<RigidBodyComponent>(entity);
					// if (rbComp) {
					// rbComp->linearVelocity = FromJoltVec3(body.GetLinearVelocity());
					// rbComp->angularVelocity = FromJoltVec3(body.GetAngularVelocity());
					// }
				}
			}
			else
			{
				std::cerr << "WARN: Failed to lock body " << bodyID.GetIndex() << " for transform sync." << std::endl;
			}
		}
		else
		{
			std::cerr << "WARN: Could not find entity mapping for active body " << bodyID.GetIndex() << std::endl;
		}
	}
}
