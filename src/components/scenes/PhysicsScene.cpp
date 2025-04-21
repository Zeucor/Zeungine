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
#include <zg/Registry.hpp>
using namespace zg::components::scenes;
using zg::components::entities::BoxShapeData;
using zg::components::entities::CapsuleShapeData;
using zg::components::entities::ConvexHullShapeData;
using zg::components::entities::MeshShapeData;
using zg::components::entities::ShapeType;
using zg::components::entities::SphereShapeData;
using zg::physics::AABB;
using zg::physics::CollisionManifold;
using zg::physics::OBB;
using zg::physics::Plane;
using zg::physics::Projection;
#define JOLT_MAX_BODIES 100000
#define JOLT_NUM_BODY_MUTEXES 10000
#define JOLT_MAX_BODY_PAIRS 1000
#define JOLT_MAX_CONTACT_CONSTRAINTS 1000
zg::components::scenes::SceneComponentCreateInfo zg::components::scenes::PhysicsSceneFactory(long double deltaTime)
{
	zg::components::scenes::SceneComponentCreateInfo info{
		.name = "PhysicsScene",
		.onAttachedFunction =
			[&, deltaTime](auto& component)
		{
			JPH::RegisterDefaultAllocator();
			JPH::Factory::sInstance = new JPH::Factory();
			JPH::RegisterTypes();
			auto& gravity = component.template make<IGravity*>("gravity");
			auto& rigidBodiesJoltID =
				component.template make<std::unordered_map<components::entities::EntityComponent*, JPH::BodyID>>("rigidBodiesJoltID");
			auto& joltIDRigidBodies =
				component.template make<std::unordered_map<JPH::BodyID, components::entities::EntityComponent*>>("joltIDRigidBodies");
			auto& collisionContacts = component.template make<std::vector<physics::CollisionManifold>>("collisionContacts");
			auto& contactListener = component.template make<ZGContactListener>("contactListener", component);
			auto& deltaTime = component.template make<long double>("deltaTime", 0);
			auto& frameduration =
				(component.template make<NANOSECONDS_DURATION>("frameduration") = NANOSECONDS_DURATION(deltaTime * NANOSECONDS::den));
			// auto& framebudget = (component.emplace<budget::ZBudget<SYS_CLOCK, NANO_TIMEPOINT, NANOSECONDS_DURATION,
			// LD_REAL>>( 											 "framebudget", frameduration, 1, false, false, "PhysicsSceneBudget"));
			component.template make<bool>("running", true);
			auto& runningMutex = component.template make<std::mutex*>("runningMutex", new std::mutex());
			auto& mTempAllocator =
				component.template make<JPH::TempAllocatorImpl*>("mTempAllocator", new JPH::TempAllocatorImpl(10 * 1024 * 1024));
			auto& mJobSystem = component.template make<JPH::JobSystemThreadPool*>(
				"mJobSystem", new JPH::JobSystemThreadPool(2048, 128, std::thread::hardware_concurrency() - 1));
			auto& mPhysicsSystem = component.template make<JPH::PhysicsSystem*>("mPhysicsSystem", new JPH::PhysicsSystem());
			auto& mBroadPhaseLayerInterface =
				component.template make<BPLayerInterfaceImpl*>("mBroadPhaseLayerInterface", new BPLayerInterfaceImpl());
			auto& mObjectVsBroadPhaseLayerFilter = component.template make<ZGObjectVsBroadPhaseLayerFilter*>(
				"mObjectVsBroadPhaseLayerFilter", new ZGObjectVsBroadPhaseLayerFilter());
			auto& mObjectLayerPairFilter =
				component.template make<ZGObjectLayerPairFilter*>("mObjectLayerPairFilter", new ZGObjectLayerPairFilter());
			mPhysicsSystem->SetContactListener(&contactListener);
			mPhysicsSystem->Init(JOLT_MAX_BODIES, JOLT_NUM_BODY_MUTEXES, JOLT_MAX_BODY_PAIRS, JOLT_MAX_CONTACT_CONSTRAINTS,
													 *mBroadPhaseLayerInterface, *mObjectVsBroadPhaseLayerFilter, *mObjectLayerPairFilter);
			if (gravity)
				mPhysicsSystem->SetGravity(JPH::Vec3(0, 0, 0));
			auto& componentHostIndexStack = component.hostIndexStack;
			auto& componentID = component.ID;
			auto& thread = component.template make<std::thread*>(
				"thread",
				new std::thread(
					[componentHostIndexStack, componentID, deltaTime]() mutable
					{
						auto scenePointer = &Registry::getScene(componentHostIndexStack);
						auto& component = scenePointer->getComponentByID(componentID);
						// do
						// {
						// 	runningMutex->lock();
						// 	if (!*runningPointer)
						// 	{
						// 		runningMutex->unlock();
						// 		break;
						// 	}
						// 	framebudget.begin();
						// 	// if (gravity)
						// 	// 	gravity->applyGravity(*this, deltaTime);
						// 	mPhysicsSystem->Update(deltaTime, 10, mTempAllocator, mJobSystem);
						// 	framebudget.end();
						// 	runningMutex->unlock();
						// 	framebudget.sleep();
						// }
						// while (true);
					}));
		},
		.onDetachedFunction =
			[&](auto& component)
		{
			auto& gravity = component.template getData<IGravity*>("gravity");
			auto& rigidBodiesJoltID =
				component.template getData<std::unordered_map<components::entities::EntityComponent*, JPH::BodyID>>("rigidBodiesJoltID");
			auto& joltIDRigidBodies =
				component.template getData<std::unordered_map<JPH::BodyID, components::entities::EntityComponent*>>("joltIDRigidBodies");
			auto& running = component.template getData<bool>("running");
			auto& mTempAllocator = component.template getData<JPH::TempAllocatorImpl*>("mTempAllocator");
			auto& mJobSystem = component.template getData<JPH::JobSystemThreadPool*>("mJobSystem");
			auto& mPhysicsSystem = component.template getData<JPH::PhysicsSystem*>("mPhysicsSystem");
			auto& runningMutex = *component.template getData<std::mutex*>("runningMutex");
			{
				std::lock_guard lock(runningMutex);
				running = false;
			}
			auto& thread = component.template getData<std::thread*>("thread");
			if (thread->joinable())
				thread->join();
			auto& scene = Registry::getScene(component.hostIndexStack);
			auto entitiesSize = scene.entities.size();
			auto entitiesData = scene.entities.data();
			for (size_t index = 0; index < entitiesSize; ++index)
			{
				auto& entity = entitiesData[index];
				entity.detachComponent("Collider");
				entity.detachComponent("RigidBody");
			}
			rigidBodiesJoltID.clear();
			joltIDRigidBodies.clear();
			JPH::UnregisterTypes();
			delete JPH::Factory::sInstance;
			JPH::Factory::sInstance = nullptr;
			delete mTempAllocator;
			delete mJobSystem;
			delete mPhysicsSystem;
			gravity = nullptr;
		},
		.onUpdateFunction =
			[&](auto& component)
		{
			auto& mPhysicsSystem = component.template getData<JPH::PhysicsSystem*>("mPhysicsSystem");

			auto& rigidBodiesJoltID =
				component.template getData<std::unordered_map<components::entities::EntityComponent*, JPH::BodyID>>("rigidBodiesJoltID");
			auto& joltIDRigidBodies =
				component.template getData<std::unordered_map<JPH::BodyID, components::entities::EntityComponent*>>("joltIDRigidBodies");
			if (!mPhysicsSystem)
				return;

			auto& body_interface = mPhysicsSystem->GetBodyInterface();

			// Iterate through *active* Jolt bodies. Inactive bodies don't need updating.
			// Note: This requires locking if accessed from multiple threads.
			// Consider processing only bodies linked to your active entities if possible.
			const JPH::BodyLockInterface& lock_interface =
				mPhysicsSystem->GetBodyLockInterfaceNoLock(); // Use NoLock if on main thread
			// const JPH::BodyLockInterface& lock_interface = mPhysicsSystem->GetBodyLockInterface(); // Use locking version
			// if needed

			JPH::BodyIDVector active_body_ids;
			mPhysicsSystem->GetActiveBodies(JPH::EBodyType::RigidBody, active_body_ids); // Get IDs of currently active bodies

			for (const auto& bodyID : active_body_ids)
			{
				// Find the corresponding Zeungine entity
				auto entityIt = joltIDRigidBodies.find(bodyID);
				if (entityIt != joltIDRigidBodies.end())
				{
					auto& rb = entityIt->second;

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
							// (*rb->position) = ToJolt<JPH::Vec3, glm::vec3>(position);
							// (*rb->rotation) = ToJolt<JPH::Quat, glm::quat>(rotation);

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
		},
		.getDataFunctions = {{std::string("BodyInterface"),
													[](auto& component) -> std::any&
													{
														try
														{
															return component.getDataReturnAny("mBodyInterface");
														}
														catch (...)
														{
														}
														auto& physicsSystem = component.template getData<JPH::PhysicsSystem*>("mPhysicsSystem");
														return component.template makeReturnAny<JPH::BodyInterface*>("mBodyInterface",
																																								&physicsSystem->GetBodyInterface());
													}}},
		.setDataFunctions =
			{{"registerRigidBody",
				[](const std::any& val, auto& component) -> void
				{
					auto& rigidBodiesJoltID =
						component.template getData<std::unordered_map<components::entities::EntityComponent*, JPH::BodyID>>(
							"rigidBodiesJoltID");
					auto& joltIDRigidBodies =
						component.template getData<std::unordered_map<JPH::BodyID, components::entities::EntityComponent*>>(
							"joltIDRigidBodies");
					const auto& valPair = std::any_cast<const JoltIDComponentPair&>(val);
					rigidBodiesJoltID[valPair.second] = valPair.first;
					joltIDRigidBodies[valPair.first] = valPair.second;
				}},
			 {"unregisterRgidBody", [](const std::any& val, auto& component) -> void
				{
					auto rigidBodyComponent = std::any_cast<components::entities::EntityComponent*>(val);
					auto& rigidBodiesJoltID =
						component.template getData<std::unordered_map<components::entities::EntityComponent*, JPH::BodyID>>(
							"rigidBodiesJoltID");
					auto& joltIDRigidBodies =
						component.template getData<std::unordered_map<JPH::BodyID, components::entities::EntityComponent*>>(
							"joltIDRigidBodies");
					if (rigidBodyComponent)
					{
						auto& joltBodyID = rigidBodyComponent->template getData<JPH::BodyID>("BodyID");
						rigidBodiesJoltID.erase(rigidBodyComponent);
						joltIDRigidBodies.erase(joltBodyID);
					}
				}}}};
	return info;
}
// void PhysicsScene::registerRigidBody(RigidBody* rigidBody)
// {
// 	auto joltBodyID = rigidBody->getJoltBodyID();
// 	rigidBodiesJoltID[rigidBody] = joltBodyID;
// 	joltIDRigidBodies[joltBodyID] = rigidBody;
// 	// std::cout << "Registered RigidBody." << std::endl;
// }
// void PhysicsScene::unregisterRigidBody(RigidBody* rigidBody)
// {
// }
// JPH::PhysicsSystem& PhysicsScene::GetJoltPhysicsSystem() { return *mPhysicsSystem; }
// const JPH::BodyLockInterface& PhysicsScene::GetBodyLockInterface()
// {
// 	return mPhysicsSystem->GetBodyLockInterfaceNoLock();
// }
// void PhysicsScene::setGravity(IGravity* gravityPointer)
// {
// 	gravity = gravityPointer;
// }
// void PhysicsScene::loop()
// {
// }
// void PhysicsScene::stepSimulation(float dt)
// {
// }
// void PhysicsScene::synchronize()
// {
// }


ZGContactListener::ZGContactListener(SceneComponent& physicsScene) : physicsScene(physicsScene) {}

CollisionManifold ZGContactListener::constructManifold(components::entities::EntityComponent* ec1,
																											 components::entities::EntityComponent* ec2,
																											 const JPH::ContactManifold& inManifold)
{
	zg::physics::CollisionManifold manifold(ec1, ec2);
	manifold.colliding = true;
	manifold.normal = ToJolt<JPH::Vec3, glm::vec3>(inManifold.mWorldSpaceNormal);
	manifold.penetrationDepth = inManifold.mPenetrationDepth;
	size_t i = 0;
	for (auto& cp : inManifold.mRelativeContactPointsOn1)
	{
		manifold.worldContactPointsOnA.push_back(
			ToJolt<JPH::Vec3, glm::vec3>(inManifold.GetWorldSpaceContactPointOn1(i++)));
		manifold.relativeContactPointsOnA.push_back(ToJolt<JPH::Vec3, glm::vec3>(cp));
	}
	i = 0;
	for (auto& cp : inManifold.mRelativeContactPointsOn2)
	{
		manifold.worldContactPointsOnB.push_back(
			ToJolt<JPH::Vec3, glm::vec3>(inManifold.GetWorldSpaceContactPointOn2(i++)));
		manifold.relativeContactPointsOnB.push_back(ToJolt<JPH::Vec3, glm::vec3>(cp));
	}
	return manifold;
}

void ZGContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2,
																			 const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
	auto& joltIDRigidBodies =
		physicsScene.template getData<std::unordered_map<JPH::BodyID, components::entities::EntityComponent*>>("joltIDRigidBodies");
	auto ec1 = joltIDRigidBodies[inBody1.GetID()];
	auto ec2 = joltIDRigidBodies[inBody2.GetID()];
	auto manifold = constructManifold(ec1, ec2, inManifold);
	ec1->template setData<CollisionManifold>("addActiveManifold", manifold);
	ec2->template setData<CollisionManifold>("addActiveManifold", manifold);
}

void ZGContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2,
																					 const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
{
	auto& joltIDRigidBodies =
		physicsScene.template getData<std::unordered_map<JPH::BodyID, components::entities::EntityComponent*>>("joltIDRigidBodies");
	auto ec1 = joltIDRigidBodies[inBody1.GetID()];
	auto ec2 = joltIDRigidBodies[inBody2.GetID()];
	ec1->template setData<components::entities::EntityComponent*>("removeActiveManifold", ec2);
	ec2->template setData<components::entities::EntityComponent*>("removeActiveManifold", ec1);
	auto manifold = constructManifold(ec1, ec2, inManifold);
	ec1->template setData<CollisionManifold>("addActiveManifold", manifold);
	ec2->template setData<CollisionManifold>("addActiveManifold", manifold);
}

// Called when two bodies stop touching.
void ZGContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
{
	auto& joltIDRigidBodies =
		physicsScene.template getData<std::unordered_map<JPH::BodyID, components::entities::EntityComponent*>>("joltIDRigidBodies");
	auto ec1 = joltIDRigidBodies[inSubShapePair.GetBody1ID()];
	auto ec2 = joltIDRigidBodies[inSubShapePair.GetBody2ID()];
	ec1->template setData<components::entities::EntityComponent*>("removeActiveManifold", ec2);
	ec2->template setData<components::entities::EntityComponent*>("removeActiveManifold", ec1);
}
