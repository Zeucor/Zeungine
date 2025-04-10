#include <unordered_set>
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/components/entities/Collider.hpp>
#include <zg/components/entities/RigidBody.hpp>
#include <zg/components/scenes/IGravity.hpp>
#include <zg/components/scenes/PhysicsScene.hpp>
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
PhysicsScene::PhysicsScene(Scene& scene) :
		ISceneComponent("PhysicsScene"), scene(scene), deltaTime(scene.window.deltaTime)
{
	std::cout << "PhysicsScene created." << std::endl;
}
void PhysicsScene::onAttached()
{
	gravity = dynamic_cast<IGravity*>(scene.getComponentByName("IGravity").get());
	if (!gravity)
	{
		throw std::runtime_error("PhysicsScene requires an IGravity component to be added before adding to a Scene");
	}
	timeAccumulator = 0.0;
	rigidBodies.clear();
	std::cout << "PhysicsScene attached." << std::endl;
}
void PhysicsScene::onUpdate()
{
	timeAccumulator += deltaTime;
	int subSteps = 0;
	auto subTimeStep = fixedTimeStep / totalSubSteps;
	while (subSteps < totalSubSteps && timeAccumulator - subTimeStep > 0)
	{
		if (usingCCD)
			stepSimulationCCD(subTimeStep);
		else
			stepSimulation(subTimeStep);
		timeAccumulator -= subTimeStep;
		subSteps++;
	}
	if (subSteps >= totalSubSteps && timeAccumulator >= fixedTimeStep)
	{
		std::cerr << "Warning: Physics simulation lagging behind real-time." << std::endl;
		timeAccumulator = std::fmod(timeAccumulator, fixedTimeStep);
	}
	synchronizeTransforms();
}
void PhysicsScene::onDetached()
{
	rigidBodies.clear();
	gravity = nullptr; // Clear pointer, don't delete if owned by scene
	std::cout << "PhysicsScene detached." << std::endl;
}
void PhysicsScene::registerRigidBody(RigidBody* rigidBody)
{
	if (std::find(rigidBodies.begin(), rigidBodies.end(), rigidBody) == rigidBodies.end())
	{
		rigidBodies.push_back(rigidBody);
		std::cout << "Registered RigidBody." << std::endl;
	}
}
void PhysicsScene::unregisterRigidBody(RigidBody* rigidBody)
{
	if (rigidBody)
	{
		auto it = std::remove(rigidBodies.begin(), rigidBodies.end(), rigidBody);
		if (it != rigidBodies.end())
		{
			rigidBodies.erase(it, rigidBodies.end());
			std::cout << "Unregistered RigidBody." << std::endl;
		}
	}
}
void PhysicsScene::stepSimulation(long double dt)
{
	// 1. Apply global forces (like gravity)
	if (gravity)
	{
		gravity->applyGravity(*this);
	}
	// 2. Integrate motion (update velocities and predict new positions/rotations)
	updateTransforms(dt);
	// 3. Collision Detection (find pairs that are colliding and store contacts)
	detectCollisions(); // Fills collisionContacts vector
	// --- 4. Resolve Collision Impulses (Velocity Correction using PGS-like approach) ---
	// This function now contains the iteration loop internally
	resolveCollisionImpulses(dt);
	// --- 5. Apply Positional Correction (Position Correction) ---
	// Apply positional correction ONCE after the impulse loop is finished
	applyPositionalCorrection();
	updateVelocities(dt);
	// 6. Clear forces for the next step
	for (auto body : rigidBodies)
	{
		if (body)
			body->clearForces();
	}
}
void PhysicsScene::stepSimulationCCD(long double dt)
{
	long double total_dt = 0;
	int ccdSubSteps = 0;
	const int maxCcdSubSteps = 1;

	if (gravity)
	{
		gravity->applyGravity(*this);
	}

	long double sub_dt = dt / (long double)maxCcdSubSteps;

	while (total_dt < dt && ccdSubSteps < maxCcdSubSteps)
	{
		total_dt += sub_dt;
		long double minTOI = total_dt;
		std::multimap<long double, TOIResult> earliestCollisions;

		// --- 1. Broadphase using Swept AABBs ---
		potentialPairs.clear();
		// Pass remainingDt to broadphase for swept test
		findPotentialCollisionPairs(total_dt);

		// --- 2. CCD Narrowphase: Find Minimum Time of Impact ---
		for (const auto& pair : potentialPairs)
		{
			auto colliderA = pair.first;
			auto colliderB = pair.second;

			// Basic checks (already done in broadphase, but good practice)
			if (!colliderA || !colliderB)
				continue;
			auto bodyA = colliderA->getOwnerRigidBody();
			auto bodyB = colliderB->getOwnerRigidBody();
			if (!bodyA || !bodyB)
				continue;
			if (bodyA->isStatic() && bodyB->isStatic())
				continue;

			ShapeType typeA = colliderA->getShapeType();
			ShapeType typeB = colliderB->getShapeType();

			TOIResult currentResult;
			currentResult.toi = total_dt; // Default

			// --- Call appropriate CCD function ---
			if (typeA == ShapeType::Box && typeB == ShapeType::Box)
			{
				currentResult = findTOIBoxBox(colliderA, colliderB, total_dt);
			}
			else if (typeA == ShapeType::Box && typeB == ShapeType::Mesh)
			{
				// Simplified check against mesh triangles
				const auto* meshData = static_cast<const MeshShapeData*>(colliderB->getColliderInfo().shapeData.get());
				// Mesh transform at t=0
				glm::mat4 meshTransform = *bodyB->transform * glm::translate(glm::mat4(1.0f), colliderB->getOffset()) *
					glm::mat4_cast(colliderB->getRotationOffset());

				if (meshData)
				{
					auto& entity = meshData->entity;
					auto& vertices = entity.positions;
					auto& indices = entity.indices;
					// Mesh body's velocity (approximation for all vertices)
					glm::vec3 meshVel = bodyB->isDynamic() ? bodyB->linearVelocity : glm::vec3(0.0f);
					// TODO: Include angular velocity contribution to vertex velocity for accuracy

					long double minMeshTOI = total_dt;
					TOIResult bestMeshTriangleResult;
					bestMeshTriangleResult.toi = total_dt;

					for (size_t triIndex = 0; triIndex < indices.size(); triIndex += 3)
					{
						glm::vec3 v0 = glm::vec3(meshTransform * glm::vec4(vertices[indices[triIndex]], 1.0f));
						glm::vec3 v1 = glm::vec3(meshTransform * glm::vec4(vertices[indices[triIndex + 1]], 1.0f));
						glm::vec3 v2 = glm::vec3(meshTransform * glm::vec4(vertices[indices[triIndex + 2]], 1.0f));
						glm::vec3 vel0 = meshVel, vel1 = meshVel, vel2 = meshVel; // Simplified velocity

						TOIResult triResult = findTOIBoxTriangle(colliderA, v0, v1, v2, vel0, vel1, vel2, colliderB, total_dt);

						if (triResult.colliding && triResult.toi < minMeshTOI)
						{
							minMeshTOI = triResult.toi;
							bestMeshTriangleResult = triResult;
						}
					}
					currentResult = bestMeshTriangleResult;
				}
			}
			else if (typeA == ShapeType::Mesh && typeB == ShapeType::Box)
			{
				// Swap roles and call findTOIBoxTriangle
				// Need to get mesh data for A and call findTOIBoxTriangle(colliderB, v0, v1, v2, ..., colliderA, total_dt)
				// ... (Implementation omitted for brevity, similar to above block) ...
			}
			// TODO: Add other shape pairs (Sphere-Sphere, Sphere-Box etc.)

			// --- Update Minimum TOI ---
			// Use a small epsilon when comparing TOIs
			if (currentResult.colliding)
			{
				earliestCollisions.insert({currentResult.toi, currentResult});
			}
		} // End loop through potential pairs

		long double ldt = 0;
		if (earliestCollisions.size())
		{

			auto it = earliestCollisions.begin();
			while (it != earliestCollisions.end())
			{
				// Get the current key we are processing
				auto currentKey = it->first;

				// Prepare a vector to hold all values for this key
				std::vector<TOIResult> valuesForKey;

				// Find the end of the range for the current key.
				// upper_bound gives the first element GREATER than currentKey.
				auto rangeEndIt = earliestCollisions.upper_bound(currentKey);

				// Iterate only through the elements matching the current key
				// (from the current iterator 'it' up to 'rangeEndIt')
				for (auto currentIt = it; currentIt != rangeEndIt; ++currentIt)
				{
					valuesForKey.push_back(currentIt->second);
				}

				// Process the key and its collected values
				for (size_t i = 0; i < valuesForKey.size(); ++i)
				{
					collisionContacts.push_back(valuesForKey[i].manifold);
				}

				float idt = currentKey - ldt;
				ldt = currentKey;
				updateTransforms(idt);
				resolveCollisionImpulses(idt);
				applyPositionalCorrection();
				updateVelocities(idt);
				collisionContacts.clear();

				it = rangeEndIt;
			}
		}
		else
		{
		_integrateFullTOI:
			ldt = minTOI;
			updateTransforms(minTOI);
			updateVelocities(minTOI);
		}
		if (!ldt)
		{
			goto _integrateFullTOI;
		}

		total_dt = std::max(total_dt, minTOI);
		// Safety break if minTOI is near zero repeatedly
		// if (minTOI < CCD_EPSILON && !earliestCollisions.empty())
		// {
		// 	// Avoid potential infinite loop if TOI is stuck at 0
		// 	// This can happen with resting contact or precision issues
		// 	// May need better handling for resting contacts (e.g., sleeping)
		// 	std::cerr << "Warning: CCD sub-step TOI near zero. Breaking loop." << std::endl;
		// 	break;
		// }
		ccdSubSteps++;

	} // End CCD sub-step loop

	// if (ccdSubSteps >= maxCcdSubSteps && remainingDt > CCD_EPSILON)
	// {
	// 	std::cerr << "Warning: Max CCD sub-steps reached. Remaining dt: " << remainingDt << std::endl;
	// }

	for (auto body : rigidBodies)
	{
		if (body)
			body->clearForces();
	}
	synchronizeTransforms();
}

void PhysicsScene::findPotentialCollisionPairs(long double dt)
{
	potentialPairs.clear();
	// if (!scene.bvh) {
	//     std::cerr << "Error: BVH not available for physics broadphase." << std::endl;
	//     // Implement N^2 fallback with Swept AABBs if desired
	//     // ... N^2 loop ...
	//     //     AABB sweptA = colliderA->getSweptWorldAABB(dt);
	//     //     AABB sweptB = colliderB->getSweptWorldAABB(dt);
	//     //     if (sweptA.overlaps(sweptB)) { potentialPairs.push_back({colliderA, colliderB}); }
	//     // ... end N^2 loop ...
	//     return;
	// }

	// Using BVH Query (Option B from previous explanation)
	std::vector<Collider*> candidates;
	// Use a set to store unique pairs, ensuring order (e.g., lower pointer first)
	std::unordered_set<std::pair<Collider*, Collider*>> uniquePairs;


	for (auto bodyAIter = rigidBodies.begin(), rigidBodiesEnd = rigidBodies.end(); bodyAIter != rigidBodiesEnd;
			 ++bodyAIter)
	{
		auto& bodyA = *bodyAIter;
		for (auto colliderA : bodyA->colliders)
		{
			if (!colliderA)
				continue;
			candidates.clear();
			AABB colliderAAABB = colliderA->getSweptWorldAABB(dt);
			for (auto bodyBIter = bodyAIter + 1; bodyBIter != rigidBodiesEnd; ++bodyBIter)
			{
				auto bodyB = *bodyBIter;
				for (auto colliderB : bodyB->colliders)
				{
					if (!colliderA)
						continue;
					AABB colliderBAABB = colliderB->getSweptWorldAABB(dt);
					if (colliderAAABB.overlaps(colliderBAABB) != AABB::Overlaps::None)
					{
						auto first = std::min(colliderA, colliderB);
						auto second = std::max(colliderA, colliderB);
						uniquePairs.insert({first, second});
					}
				}
			}
			// scene.bvh->queryAABB(queryAABB, candidates); // Assumes BVH has queryAABB method
			// for (auto colliderB : candidates) {
			//     if (!colliderB || colliderA == colliderB) continue;
			//     auto bodyB = colliderB->getOwnerRigidBody();
			//     if (!bodyB) continue;
			//     // Avoid static-static (though bodyA is dynamic here) and self-collision
			//     if (bodyA->isStatic() && bodyB->isStatic()) continue;
			//     // Ensure consistent pair ordering for uniqueness check
			//     auto first = std::min(colliderA, colliderB);
			//     auto second = std::max(colliderA, colliderB);
			//     uniquePairs.insert({first, second}); // Add to set (duplicates ignored)
			// }
		}
	}
	potentialPairs.assign(uniquePairs.begin(), uniquePairs.end());
}

void PhysicsScene::applyPositionalCorrection()
{
	for (const CollisionMannifold& manifold : collisionContacts) // Iterate over stored contacts
	{
		if (manifold.penetrationDepth <= 0)
		{
			continue;
		}
		auto colliderA = manifold.colliderA;
		auto colliderB = manifold.colliderB;
		auto bodyA = colliderA ? colliderA->getOwnerRigidBody() : nullptr;
		auto bodyB = colliderB ? colliderB->getOwnerRigidBody() : nullptr;

		if (!bodyA || !bodyB || !colliderA || !colliderB)
			continue;
		if (bodyA->isStatic() && bodyB->isStatic())
			continue; // No correction needed
		if (colliderA->getIsSensor() || colliderB->getIsSensor())
			continue; // Sensors don't get corrected

		glm::vec3 correctionVector;
		if (bodyA->isDynamic() && bodyB->isDynamic())
		{
			correctionVector = manifold.normal * manifold.penetrationDepth / 2.f;
		}
		else
		{
			correctionVector = manifold.normal * manifold.penetrationDepth;
		}

		// Apply the correction
		if (bodyA->isDynamic() && bodyA->transform)
		{
			bodyA->translate(-correctionVector * bodyA->inverseMass);
		}
		if (bodyB->isDynamic() && bodyB->transform)
		{
			bodyB->translate(correctionVector * bodyB->inverseMass);
		}
		continue;
	}
	// Removed return value as it wasn't used in the loop structure anymore
}
void PhysicsScene::updateTransforms(long double dt)
{
	float fdt = static_cast<float>(dt); // Use float for glm calculations

	for (auto body : rigidBodies)
	{
		if (!body || !body->isDynamic() || !body->transform)
			continue; // Skip non-dynamic, null, or bodies without transform

		// --- Linear Motion ---

		// Apply velocity constraints
		if (body->info.freezeVelocityAxes.x)
			body->linearVelocity.x = 0;
		if (body->info.freezeVelocityAxes.y)
			body->linearVelocity.y = 0;
		if (body->info.freezeVelocityAxes.z)
			body->linearVelocity.z = 0;

		// Update position (using the RigidBody's translate method)
		body->translate(body->linearVelocity * fdt);


		// --- Angular Motion ---
		// TODO: Needs Inverse Inertia Tensor implementation
		// glm::vec3 angularAcceleration = body->inverseInertiaTensor * body->torqueAccumulator; // Correct calculation

		// Apply rotation constraints
		if (body->info.freezeRotationAxes.x)
			body->angularVelocity.x = 0;
		if (body->info.freezeRotationAxes.y)
			body->angularVelocity.y = 0;
		if (body->info.freezeRotationAxes.z)
			body->angularVelocity.z = 0;

		// Update rotation (using the RigidBody's setRotation method)
		if (glm::length2(body->angularVelocity) > 1e-9f) // Use length2 for efficiency, adjust threshold
		{
			float angle = glm::length(body->angularVelocity) * fdt;
			glm::vec3 axis = glm::normalize(body->angularVelocity);
			glm::quat rotationDelta = glm::angleAxis(angle, axis);
			// Apply delta: newRotation = delta * oldRotation
			body->setRotation(rotationDelta * body->getRotation());
		}
	}
}
void PhysicsScene::updateVelocities(long double dt)
{
	float fdt = static_cast<float>(dt); // Use float for glm calculations

	for (auto body : rigidBodies)
	{
		if (!body || !body->isDynamic() || !body->transform)
			continue; // Skip non-dynamic, null, or bodies without transform

		// --- Linear Motion ---
		glm::vec3 linearAcceleration = body->forceAccumulator * body->inverseMass;
		body->linearVelocity += linearAcceleration * fdt;
		// Apply damping: dv = -damping * v * dt => v_new = v * (1 - damping * dt)
		// More stable damping: v_new = v / (1 + damping * dt) or pow(1-damping, dt) is also common
		body->linearVelocity *= std::pow(1.0f - body->info.linearDamping, fdt);

		glm::vec3 angularAcceleration = {0.0f, 0.0f, 0.0f}; // Placeholder
		body->angularVelocity += angularAcceleration * fdt;
		body->angularVelocity *= std::pow(1.0f - body->info.angularDamping, fdt);
	}
}
void PhysicsScene::detectCollisions()
{
	potentialPairs.clear();
	collisionContacts.clear();

	// --- 1. Broadphase (N^2 AABB Check) ---
	for (size_t i = 0; i < rigidBodies.size(); ++i)
	{
		auto bodyA = rigidBodies[i];
		if (!bodyA || !bodyA->transform || bodyA->colliders.empty())
			continue;

		for (size_t j = i + 1; j < rigidBodies.size(); ++j)
		{
			auto bodyB = rigidBodies[j];
			if (!bodyB || !bodyB->transform || bodyB->colliders.empty())
				continue;
			if (bodyA->isStatic() && bodyB->isStatic())
				continue;

			for (auto colliderA : bodyA->colliders)
			{
				if (!colliderA)
					continue;
				for (auto colliderB : bodyB->colliders)
				{
					if (!colliderB)
						continue;
					if (colliderA->getWorldAABB().overlaps(colliderB->getWorldAABB()) != AABB::Overlaps::None)
					{
						potentialPairs.push_back({colliderA, colliderB});
					}
				}
			}
		}
	}

	// --- 2. Narrowphase ---
	for (const auto& pair : potentialPairs)
	{
		auto colliderA = pair.first;
		auto colliderB = pair.second;

		ShapeType typeA = colliderA->getShapeType();
		ShapeType typeB = colliderB->getShapeType();

		CollisionMannifold manifold(colliderA, colliderB); // Create manifold for potential collision

		// --- Box-Box ---
		if (typeA == ShapeType::Box && typeB == ShapeType::Box)
		{
			if (performSATBoxBox(colliderA, colliderB, manifold))
			{
				collisionContacts.push_back(manifold);
			}
		}
		// --- Box-Mesh ---
		else if (typeA == ShapeType::Box && typeB == ShapeType::Mesh)
		{
			// Iterate through mesh triangles (inefficient without BVH)
			const auto* meshData = static_cast<const MeshShapeData*>(colliderB->getColliderInfo().shapeData.get());
			const glm::mat4* meshTransformPtr = colliderB->getTransform();
			if (meshData && meshTransformPtr)
			{
				const glm::mat4& meshTransform = *meshTransformPtr;
				auto& entity = meshData->entity;
				auto& vertices = entity.positions;
				auto& indices = entity.indices;
				for (size_t triIndex = 0; triIndex < indices.size(); triIndex += 3)
				{
					// Get triangle vertices in world space
					glm::vec3 v0 = glm::vec3(meshTransform * glm::vec4(vertices[indices[triIndex]], 1.0f));
					glm::vec3 v1 = glm::vec3(meshTransform * glm::vec4(vertices[indices[triIndex + 1]], 1.0f));
					glm::vec3 v2 = glm::vec3(meshTransform * glm::vec4(vertices[indices[triIndex + 2]], 1.0f));

					// Perform Box-Triangle SAT check
					// Pass meshCollider (colliderB) for material properties etc.
					if (performSATBoxTriangle(colliderA, v0, v1, v2, colliderB, manifold))
					{
						// If collision, add manifold (manifold is updated by performSATBoxTriangle)
						// We might get multiple manifolds for one Box-Mesh pair if box hits multiple triangles.
						// This might need refinement (e.g., finding deepest penetration).
						// For now, just add each colliding triangle's manifold.
						collisionContacts.push_back(manifold);
						// Optimization: Could potentially break after first hit if only one contact point is needed,
						// but checking all triangles might be necessary for deeper penetrations.
					}
				}
			}
		}
		// --- Mesh-Box (handle swapped order) ---
		else if (typeA == ShapeType::Mesh && typeB == ShapeType::Box)
		{
			// Swap roles and call the Box-Mesh logic
			const auto* meshData = static_cast<const MeshShapeData*>(colliderA->getColliderInfo().shapeData.get());
			const glm::mat4* meshTransformPtr = colliderA->getTransform();
			if (meshData && meshTransformPtr)
			{
				const glm::mat4& meshTransform = *meshTransformPtr;
				auto& entity = meshData->entity;
				auto& vertices = entity.positions;
				auto& indices = entity.indices;
				for (size_t triIndex = 0; triIndex < indices.size(); triIndex += 3)
				{
					glm::vec3 v0 = glm::vec3(meshTransform * glm::vec4(vertices[indices[triIndex]], 1.0f));
					glm::vec3 v1 = glm::vec3(meshTransform * glm::vec4(vertices[indices[triIndex + 1]], 1.0f));
					glm::vec3 v2 = glm::vec3(meshTransform * glm::vec4(vertices[indices[triIndex + 2]], 1.0f));

					// Create manifold with swapped colliders for correct resolution later
					CollisionMannifold swappedManifold(colliderB, colliderA);
					if (performSATBoxTriangle(colliderB, v0, v1, v2, colliderA, swappedManifold))
					{
						collisionContacts.push_back(swappedManifold);
					}
				}
			}
		}
		// TODO: Add other collision pairs (Sphere-Sphere, Sphere-Box, Sphere-Mesh, Mesh-Mesh...)
	}
}
bool PhysicsScene::performSATBoxBox(Collider* boxA, Collider* boxB, CollisionMannifold& manifold)
{
	// --- Input Validation ---
	if (!boxA || boxA->getShapeType() != ShapeType::Box || !boxB || boxB->getShapeType() != ShapeType::Box)
	{
		return false; // Invalid input
	}
	auto bodyA = boxA->getOwnerRigidBody();
	auto bodyB = boxB->getOwnerRigidBody();
	if (!bodyA || !bodyB)
	{
		// Need rigid bodies to get transforms and positions
		std::cerr << "Warning: SATBoxBox requires colliders to have RigidBody owners." << std::endl;
		return false;
	}


	// --- SAT Initialization ---
	float minPenetration = std::numeric_limits<float>::infinity();
	glm::vec3 collisionNormal(0.0f);
	bool separatingAxisFound = false; // Flag to track if we found *any* separating axis

	// --- Get Axes to Test ---
	// Get world-aligned axes for both boxes (3 each)
	std::vector<glm::vec3> axesA = getBoxWorldAxes(boxA);
	std::vector<glm::vec3> axesB = getBoxWorldAxes(boxB);

	// Combine axes: 3 from A, 3 from B, up to 9 from cross products
	std::vector<glm::vec3> testAxes;
	testAxes.reserve(15); // 3 + 3 + 9 = 15 potential axes
	testAxes.insert(testAxes.end(), axesA.begin(), axesA.end());
	testAxes.insert(testAxes.end(), axesB.begin(), axesB.end());

	// Generate cross product axes (edge-edge directions)
	for (const auto& axisA : axesA)
	{
		for (const auto& axisB : axesB)
		{
			glm::vec3 crossAxis = glm::cross(axisA, axisB);
			// Check for non-zero length (handles parallel axes) using squared length for efficiency
			if (glm::length2(crossAxis) > 1e-8f)
			{ // Use a small epsilon
				testAxes.push_back(glm::normalize(crossAxis));
			}
		}
	}

	// --- Test Projections on Each Axis ---
	for (const auto& axis : testAxes)
	{
		// Check for degenerate axes (should be rare after normalization, but safe)
		if (glm::length2(axis) < 1e-8f)
			continue;

		float minA, maxA, minB, maxB;
		projectBoxOntoAxis(boxA, axis, minA, maxA);
		projectBoxOntoAxis(boxB, axis, minB, maxB);

		// Calculate overlap on this axis
		// overlap = min(maxA, maxB) - max(minA, minB)
		float currentOverlap = glm::min(maxA, maxB) - glm::max(minA, minB);

		// --- Check for Separation ---
		if (currentOverlap < 0.0f)
		{
			// Found a separating axis! No collision possible.
			separatingAxisFound = true;
			break; // Exit the loop early
		}

		// --- Update Minimum Penetration ---
		// **CRITICAL CHANGE:** Update penetration based on *any* axis that shows overlap.
		// The axis with the *smallest* overlap corresponds to the minimum penetration depth.
		if (currentOverlap < minPenetration)
		{
			minPenetration = currentOverlap;
			collisionNormal = axis; // Store the axis associated with this minimum overlap
		}
	}

	// --- Final Result ---
	if (separatingAxisFound)
	{
		return false; // No collision because a separating axis was found
	}

	// If no separating axis was found after checking all axes, the boxes are colliding.
	// We should have a valid collisionNormal and minPenetration by now.

	// --- Finalize Manifold ---
	// Ensure the normal points from B to A (or consistently, e.g., A to B)
	// Get approximate centers using RigidBody position + Collider offset
	glm::vec3 centerA = bodyA->getPosition() + (bodyA->getRotation() * boxA->getOffset()); // Apply rotation to offset
	glm::vec3 centerB = bodyB->getPosition() + (bodyB->getRotation() * boxB->getOffset()); // Apply rotation to offset
	glm::vec3 directionBA = centerA - centerB; // Vector from B's center to A's center

	// Flip the normal if it's pointing in the "wrong" direction relative to the centers
	if (glm::dot(collisionNormal, directionBA) < 0.0f)
	{
		collisionNormal = -collisionNormal; // Ensure normal points roughly from B towards A
	}

	manifold.colliding = true;
	manifold.normal = glm::normalize(collisionNormal); // Ensure normal is unit length
	manifold.penetrationDepth = minPenetration;
	// manifold.colliderA = boxA; // Already set in detectCollisions
	// manifold.colliderB = boxB; // Already set in detectCollisions

	// TODO: Calculate Contact Points (Advanced)
	// This often involves finding the features (vertices, edges, faces) that correspond
	// to the minimum penetration axis and then clipping them against each other.
	// For box-box, common methods include Sutherland-Hodgman clipping or finding
	// the intersection of the relevant features.
	// As a placeholder, you could use the point on boxA closest to boxB along the normal,
	// or the midpoint of the overlap interval projected back onto the boxes.
	// Example placeholder: Contact point approx center of overlap
	// glm::vec3 contactPoint = centerB + collisionNormal * (glm::length(centerA - centerB) - minPenetration * 0.5f);
	// manifold.contactPoints.push_back(contactPoint);

	return true; // Collision detected
}
const float SAT_EPSILON = 1e-6f;

std::vector<glm::vec3> PhysicsScene::getBoxWorldAxesInternal(Collider* boxCollider)
{
	auto body = boxCollider ? boxCollider->getOwnerRigidBody() : nullptr;
	if (!body)
		return {};
	glm::mat3 rotation = glm::toMat3(body->getOrientation());
	return {rotation * glm::vec3(1.0f, 0.0f, 0.0f), rotation * glm::vec3(0.0f, 1.0f, 0.0f),
					rotation * glm::vec3(0.0f, 0.0f, 1.0f)};
}

void PhysicsScene::projectBoxOntoAxisInternal(Collider* boxCollider, const glm::vec3& axis, float& minProj,
																							float& maxProj)
{
	minProj = std::numeric_limits<float>::infinity();
	maxProj = -std::numeric_limits<float>::infinity();
	if (!boxCollider)
		return;
	auto body = boxCollider->getOwnerRigidBody();
	if (!body)
		return;
	auto shape = boxCollider->getColliderInfo().shapeData.get();
	auto& halfExtents = dynamic_cast<BoxShapeData*>(shape)->halfExtents;
	auto worldOffset = body->getOrientation() * boxCollider->getOffset();
	auto center = body->getPosition() + worldOffset;
	std::vector<glm::vec3> worldAxes = getBoxWorldAxesInternal(boxCollider);
	if (worldAxes.size() != 3)
		return;
	float projectedCenter = glm::dot(axis, center);
	float radius = std::abs(glm::dot(axis, worldAxes[0])) * halfExtents.x +
		std::abs(glm::dot(axis, worldAxes[1])) * halfExtents.y + std::abs(glm::dot(axis, worldAxes[2])) * halfExtents.z;
	minProj = projectedCenter - radius;
	maxProj = projectedCenter + radius;
}

void PhysicsScene::projectTriangleOntoAxisInternal(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
																									 const glm::vec3& axis, float& minProj, float& maxProj)
{
	float p0 = glm::dot(axis, v0);
	float p1 = glm::dot(axis, v1);
	float p2 = glm::dot(axis, v2);
	minProj = std::min({p0, p1, p2});
	maxProj = std::max({p0, p1, p2});
}

std::vector<glm::vec3> PhysicsScene::getBoxFaceVerticesInternal(Collider* boxCollider, const glm::vec3& faceNormalWorld)
{
	std::vector<glm::vec3> vertices;
	if (!boxCollider)
		return vertices;
	vertices.reserve(4);
	auto body = boxCollider->getOwnerRigidBody();
	if (!body)
		return vertices;
	auto shape = boxCollider->getColliderInfo().shapeData.get();
	auto& halfExtents = dynamic_cast<BoxShapeData*>(shape)->halfExtents;
	auto worldOffset = body->getOrientation() * boxCollider->getOffset();
	auto center = body->getPosition() + worldOffset;
	std::vector<glm::vec3> worldAxes = getBoxWorldAxesInternal(boxCollider);
	if (worldAxes.size() != 3)
		return vertices;
	int bestAxis = -1;
	float maxDot = -std::numeric_limits<float>::infinity();
	float sign = 1.0f;
	for (int i = 0; i < 3; ++i)
	{
		float dot = glm::dot(faceNormalWorld, worldAxes[i]);
		if (dot > maxDot)
		{
			maxDot = dot;
			bestAxis = i;
			sign = 1.0f;
		}
		dot = glm::dot(faceNormalWorld, -worldAxes[i]);
		if (dot > maxDot)
		{
			maxDot = dot;
			bestAxis = i;
			sign = -1.0f;
		}
	}
	if (bestAxis == -1)
		return vertices;
	glm::vec3 faceCenter = center + worldAxes[bestAxis] * (sign * halfExtents[bestAxis]);
	int axisU = (bestAxis + 1) % 3;
	int axisV = (bestAxis + 2) % 3;
	vertices.push_back(faceCenter + worldAxes[axisU] * halfExtents[axisU] + worldAxes[axisV] * halfExtents[axisV]);
	vertices.push_back(faceCenter - worldAxes[axisU] * halfExtents[axisU] + worldAxes[axisV] * halfExtents[axisV]);
	vertices.push_back(faceCenter - worldAxes[axisU] * halfExtents[axisU] - worldAxes[axisV] * halfExtents[axisV]);
	vertices.push_back(faceCenter + worldAxes[axisU] * halfExtents[axisU] - worldAxes[axisV] * halfExtents[axisV]);
	return vertices;
}

glm::vec3 PhysicsScene::getBoxFaceCenterInternal(Collider* boxCollider, const glm::vec3& faceNormalWorld)
{
	if (!boxCollider)
		return glm::vec3(0.0f);
	auto body = boxCollider->getOwnerRigidBody();
	if (!body)
		return glm::vec3(0.0f);
	auto shape = boxCollider->getColliderInfo().shapeData.get();
	auto& halfExtents = dynamic_cast<BoxShapeData*>(shape)->halfExtents;
	auto worldOffset = body->getOrientation() * boxCollider->getOffset();
	auto center = body->getPosition() + worldOffset;
	std::vector<glm::vec3> worldAxes = getBoxWorldAxesInternal(boxCollider);
	if (worldAxes.size() != 3)
		return center;
	int bestAxis = -1;
	float maxDot = -std::numeric_limits<float>::infinity();
	float sign = 1.0f;
	for (int i = 0; i < 3; ++i)
	{
		float dot = glm::dot(faceNormalWorld, worldAxes[i]);
		if (dot > maxDot)
		{
			maxDot = dot;
			bestAxis = i;
			sign = 1.0f;
		}
		dot = glm::dot(faceNormalWorld, -worldAxes[i]);
		if (dot > maxDot)
		{
			maxDot = dot;
			bestAxis = i;
			sign = -1.0f;
		}
	}
	if (bestAxis == -1)
		return center;
	return center + worldAxes[bestAxis] * (sign * halfExtents[bestAxis]);
}

std::vector<glm::vec3> PhysicsScene::getBestBoxFaceInternal(Collider* boxCollider, const glm::vec3& directionWorld)
{
	if (!boxCollider)
		return {};
	std::vector<glm::vec3> worldAxes = getBoxWorldAxesInternal(boxCollider);
	if (worldAxes.size() != 3)
		return {};
	int bestAxis = -1;
	float maxDot = -std::numeric_limits<float>::infinity();
	glm::vec3 bestFaceNormal;
	for (int i = 0; i < 3; ++i)
	{
		float dot = glm::dot(directionWorld, worldAxes[i]);
		if (dot > maxDot)
		{
			maxDot = dot;
			bestAxis = i;
			bestFaceNormal = worldAxes[i];
		}
		dot = glm::dot(directionWorld, -worldAxes[i]);
		if (dot > maxDot)
		{
			maxDot = dot;
			bestAxis = i;
			bestFaceNormal = -worldAxes[i];
		}
	}
	if (bestAxis != -1)
		return getBoxFaceVerticesInternal(boxCollider, bestFaceNormal);
	return {};
}

void PhysicsScene::projectInterval(const glm::vec3* vertices, int numVertices, const glm::mat4& transform,
																	 const glm::vec3& axis, float& minProj, float& maxProj)
{
	minProj = std::numeric_limits<float>::infinity();
	maxProj = -std::numeric_limits<float>::infinity();
	for (int i = 0; i < numVertices; ++i)
	{
		glm::vec4 worldVertex4 = transform * glm::vec4(vertices[i], 1.0f);
		float projection = glm::dot(glm::vec3(worldVertex4), axis);
		minProj = std::min(minProj, projection);
		maxProj = std::max(maxProj, projection);
	}
}

// Project Box onto axis (using precomputed world axes and center)
void PhysicsScene::projectBox(const glm::vec3& center, const glm::vec3& halfExtents,
															const std::vector<glm::vec3>& worldAxes, const glm::vec3& axis, float& minProj,
															float& maxProj)
{
	// Project center onto axis
	float projectedCenter = glm::dot(center, axis);
	// Calculate radius of projection (sum of projected half-axes)
	float radius = std::abs(glm::dot(axis, worldAxes[0])) * halfExtents.x +
		std::abs(glm::dot(axis, worldAxes[1])) * halfExtents.y + std::abs(glm::dot(axis, worldAxes[2])) * halfExtents.z;
	minProj = projectedCenter - radius;
	maxProj = projectedCenter + radius;
}

// Project Triangle onto axis
void PhysicsScene::projectTriangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const glm::vec3& axis,
																	 float& minProj, float& maxProj)
{
	float p0 = glm::dot(v0, axis);
	float p1 = glm::dot(v1, axis);
	float p2 = glm::dot(v2, axis);
	minProj = std::min({p0, p1, p2});
	maxProj = std::max({p0, p1, p2});
}


// Calculate interval overlap and separation rate for Swept SAT
// Returns true if it's potentially a separating axis based on current state and linear velocity
bool PhysicsScene::testAxisSwept(float minA, float maxA, float minB, float maxB,
																 /* Projections at t=0 */ float projectedRelVel,
																 /* Dot product of relative velocity and axis */ long double dt, long double& tEnter,
																 /* Output: Time when overlap starts along this axis */
																 long double& tExit /* Output: Time when overlap ends along this axis */)
{
	tEnter = 0.0L;
	tExit = dt; // Assume overlap for the whole interval initially if converging

	// Calculate separation distance at t=0
	float separation0 = (minA < minB) ? (minB - maxA) : (minA - maxB);

	if (std::abs(projectedRelVel) < SAT_EPSILON)
	{
		// Relative velocity along axis is near zero (parallel movement or stationary)
		if (separation0 > SAT_EPSILON)
		{
			// Separated and not converging -> Separating axis for the whole interval
			return true; // Is a separating axis
		}
		else
		{
			// Overlapping and not moving relative to each other along axis
			// They overlap for the whole interval [0, dt] along this axis
			tEnter = 0.0L;
			tExit = dt;
			return false; // Not a separating axis
		}
	}

	// Calculate times of entering and exiting overlap along this axis
	tEnter = static_cast<long double>(separation0 / -projectedRelVel); // Time to close the gap
	float separation1 = (maxA < maxB) ? (maxB - minA) : (maxA - minB); // Distance between furthest points
	tExit = static_cast<long double>(separation1 / -projectedRelVel); // Time to fully separate after initial contact

	// Ensure correct order if velocity is positive (moving apart)
	if (projectedRelVel > 0)
	{
		std::swap(tEnter, tExit);
	}

	// Clamp times to the interval [0, dt]
	tEnter = std::max(0.0L, tEnter);
	tExit = std::min(dt, tExit);


	// If the time interval of overlap [tEnter, tExit] is invalid (tEnter >= tExit)
	// OR if the entry time is beyond our simulation interval dt,
	// then this axis guarantees separation for the interval [0, dt].
	if (tEnter >= tExit - CCD_EPSILON || tEnter >= dt - CCD_EPSILON)
	{
		return true; // Is a separating axis for the interval [0, dt]
	}

	// Otherwise, overlap occurs along this axis during [tEnter, tExit]
	return false; // Not a separating axis for the whole interval
}

/**
 * @brief Calculates Time of Impact (TOI) between two moving Boxes using Swept AABB checks
 * @details Focuses on linear relative velocity. Rotational effects are approximated.
 * @param boxColliderA First box collider.
 * @param boxColliderB Second box collider.
 * @param dt Time interval to check for collision.
 * @return TOIResult Containing toi, collision status, and manifold at toi.
 */
TOIResult PhysicsScene::findTOIBoxBox(Collider* boxColliderA, Collider* boxColliderB, long double dt)
{
	TOIResult result;
	result.toi = static_cast<float>(dt); // Initialize TOI to full interval
	result.colliding = false;
	// Initialize manifold with colliders
	result.manifold = CollisionMannifold(boxColliderA, boxColliderB);
	result.manifold.colliding = false; // Explicitly set manifold colliding state

	// Use float for calculations
	float fdt = static_cast<float>(dt);
	if (fdt <= 0.0f)
		return result; // No time interval

	// Get rigid bodies and velocities
	RigidBody* rbA = boxColliderA->getOwnerRigidBody();
	RigidBody* rbB = boxColliderB->getOwnerRigidBody();
	glm::vec3 velA = (rbA) ? rbA->linearVelocity : glm::vec3(0.0f);
	glm::vec3 velB = (rbB) ? rbB->linearVelocity : glm::vec3(0.0f);
	glm::vec3 v = velA - velB; // Relative velocity

	// Get initial world AABBs
	AABB a = boxColliderA->getWorldAABB();
	AABB b = boxColliderB->getWorldAABB();

	// --- Calculate time intervals for overlap along each axis ---
	float t_enter = 0.0f;
	float t_exit = fdt;
	int collision_axis = -1;
	const float epsilon = 1e-7f;

	for (int i = 0; i < 3; ++i)
	{
		float v_i = v[i];
		float a_min_i = a._min[i];
		float a_max_i = a._max[i];
		float b_min_i = b._min[i];
		float b_max_i = b._max[i];

		if (std::abs(v_i) < epsilon)
		{
			if (a_max_i < b_min_i || b_max_i < a_min_i)
			{
				return result; // Early exit: separated and not approaching
			}
		}
		else
		{
			float t1 = (b_min_i - a_max_i) / v_i;
			float t2 = (b_max_i - a_min_i) / v_i;
			float t_near = std::min(t1, t2);
			float t_far = std::max(t1, t2);

			if (t_near > t_enter)
			{
				t_enter = t_near;
				collision_axis = i;
			}
			t_exit = std::min(t_exit, t_far);

			if (t_enter >= t_exit || t_enter >= fdt || t_exit <= 0.0f)
			{
				return result; // Early exit: No collision within the valid time frame
			}
		}
	}

	// --- Collision Check and Manifold Calculation ---
	if (t_enter >= 0.0f && t_enter < fdt)
	{
		// Handle initial overlap case to find collision axis if needed
		if (t_enter < epsilon && collision_axis == -1)
		{
			float min_overlap = std::numeric_limits<float>::max();
			glm::vec3 centers_diff = (a._min + a._max) * 0.5f - (b._min + b._max) * 0.5f;
			glm::vec3 half_extentsA = (a._max - a._min) * 0.5f;
			glm::vec3 half_extentsB = (b._max - b._min) * 0.5f;

			for (int i = 0; i < 3; ++i)
			{
				float overlap = (half_extentsA[i] + half_extentsB[i]) - std::abs(centers_diff[i]);
				if (overlap < epsilon)
				{
					return result;
				} // Should not happen if we got here
				if (overlap < min_overlap)
				{
					min_overlap = overlap;
					collision_axis = i;
				}
			}
			result.manifold.penetrationDepth = min_overlap;
		}

		if (collision_axis == -1)
		{
			return result;
		} // Should have an axis by now

		// --- Collision Confirmed ---
		result.colliding = true; // TOI result indicates collision
		result.toi = t_enter;
		result.manifold.colliding = true; // Manifold is now valid

		// --- Calculate Normal ---
		result.manifold.normal = glm::vec3(0.0f);
		float v_collision_axis = v[collision_axis];
		if (std::abs(v_collision_axis) < epsilon && t_enter < epsilon)
		{
			glm::vec3 centerA = (a._min + a._max) * 0.5f;
			glm::vec3 centerB = (b._min + b._max) * 0.5f;
			float center_diff = centerA[collision_axis] - centerB[collision_axis];
			result.manifold.normal[collision_axis] = (center_diff >= 0.0f) ? 1.0f : -1.0f;
		}
		else
		{
			result.manifold.normal[collision_axis] = (v_collision_axis >= 0.0f) ? 1.0f : -1.0f;
		}

		// --- Calculate Contact Points (Corners of Overlap Face) ---
		// 1. Calculate AABBs at TOI
		glm::vec3 displacementA = velA * (float)result.toi;
		glm::vec3 displacementB = velB * (float)result.toi;
		AABB aabbA_toi = {a._min + displacementA, a._max + displacementA};
		AABB aabbB_toi = {b._min + displacementB, b._max + displacementB};

		// 2. Determine coordinate of the contact plane along the normal axis
		float contact_coord;
		if (result.manifold.normal[collision_axis] > 0.0f)
		{ // Normal positive
			contact_coord = (aabbA_toi._max[collision_axis] + aabbB_toi._min[collision_axis]) * 0.5f;
		}
		else
		{ // Normal negative
			contact_coord = (aabbA_toi._min[collision_axis] + aabbB_toi._max[collision_axis]) * 0.5f;
		}

		// 3. Find the overlap rectangle on the plane perpendicular to the normal
		// Determine the two perpendicular axes indices (j, k)
		int j_axis = (collision_axis + 1) % 3;
		int k_axis = (collision_axis + 2) % 3;

		// Calculate overlap interval on axis j
		float overlap_min_j = std::max(aabbA_toi._min[j_axis], aabbB_toi._min[j_axis]);
		float overlap_max_j = std::min(aabbA_toi._max[j_axis], aabbB_toi._max[j_axis]);

		// Calculate overlap interval on axis k
		float overlap_min_k = std::max(aabbA_toi._min[k_axis], aabbB_toi._min[k_axis]);
		float overlap_max_k = std::min(aabbA_toi._max[k_axis], aabbB_toi._max[k_axis]);

		// Clear previous points (if any) and reserve space
		result.manifold.contactPoints.clear();


		// Check if there is actual overlap area (not just line or point contact)
		// Add points only if the overlap intervals are valid (min <= max)
		if (overlap_min_j <= overlap_max_j && overlap_min_k <= overlap_max_k)
		{
			result.manifold.contactPoints.reserve(4); // Reserve space for up to 4 corners

			// 4. Create the corner points of the overlap rectangle
			glm::vec3 p1, p2, p3, p4;

			// Assign coordinates based on collision_axis, j_axis, k_axis
			p1[collision_axis] = contact_coord;
			p1[j_axis] = overlap_min_j;
			p1[k_axis] = overlap_min_k;

			p2[collision_axis] = contact_coord;
			p2[j_axis] = overlap_max_j;
			p2[k_axis] = overlap_min_k;

			p3[collision_axis] = contact_coord;
			p3[j_axis] = overlap_min_j;
			p3[k_axis] = overlap_max_k;

			p4[collision_axis] = contact_coord;
			p4[j_axis] = overlap_max_j;
			p4[k_axis] = overlap_max_k;

			// Add the points to the manifold's vector
			result.manifold.contactPoints.push_back(p1);
			// Add others only if distinct (handles line/point contacts implicitly)
			if (std::abs(overlap_max_j - overlap_min_j) > epsilon)
			{
				result.manifold.contactPoints.push_back(p2);
			}
			if (std::abs(overlap_max_k - overlap_min_k) > epsilon)
			{
				result.manifold.contactPoints.push_back(p3);
			}
			// Add p4 only if both dimensions have size
			if (std::abs(overlap_max_j - overlap_min_j) > epsilon && std::abs(overlap_max_k - overlap_min_k) > epsilon)
			{
				result.manifold.contactPoints.push_back(p4);
			}
			// Ensure at least one point is added if overlap exists
			if (result.manifold.contactPoints.empty())
			{
				result.manifold.contactPoints.push_back(p1);
			}
		}
		else
		{
			// If overlap is degenerate (line or point), calculate the single point
			// which is the center of the degenerate overlap.
			glm::vec3 center_point;
			center_point[collision_axis] = contact_coord;
			center_point[j_axis] = (overlap_min_j + overlap_max_j) * 0.5f; // Center of j-interval
			center_point[k_axis] = (overlap_min_k + overlap_max_k) * 0.5f; // Center of k-interval
			result.manifold.contactPoints.push_back(center_point);
		}


		// Calculate penetration depth if it's an initial overlap
		if (result.toi < epsilon)
		{
			if (result.manifold.penetrationDepth == 0.0f)
			{ // Recalculate if not done above
				glm::vec3 centers_diff = (a._min + a._max) * 0.5f - (b._min + b._max) * 0.5f;
				glm::vec3 half_extentsA = (a._max - a._min) * 0.5f;
				glm::vec3 half_extentsB = (b._max - b._min) * 0.5f;
				float sum_extents = half_extentsA[collision_axis] + half_extentsB[collision_axis];
				result.manifold.penetrationDepth = std::max(0.0f, sum_extents - std::abs(centers_diff[collision_axis]));
			}
		}
		else
		{
			result.manifold.penetrationDepth = 0.0f;
		}
	}
	else
	{
		// No collision detected within the time interval [0, fdt)
		result.colliding = false;
		result.manifold.colliding = false;
		// Ensure manifold is cleared if no collision
		result.manifold.normal = glm::vec3(0.0f);
		result.manifold.penetrationDepth = 0.0f;
		result.manifold.contactPoints.clear();
	}

	return result;
}

/**
 * @brief Calculates Time of Impact (TOI) between a moving Box and a moving/static Triangle using Swept SAT.
 * @details Highly simplified, focuses on linear relative velocity between box center and triangle centroid.
 * @param boxCollider The box collider.
 * @param triV0, triV1, triV2 World-space vertices of the triangle at t=0.
 * @param triVel0, triVel1, triVel2 World-space velocities of triangle vertices (use body velocity if rigid).
 * @param meshCollider The collider the triangle belongs to (for material properties etc.).
 * @param dt Time interval to check for collision.
 * @return TOIResult Containing toi, collision status, and manifold at toi.
 */
TOIResult PhysicsScene::findTOIBoxTriangle(Collider* boxCollider, const glm::vec3& triV0, const glm::vec3& triV1,
																					 const glm::vec3& triV2, const glm::vec3& triVel0, const glm::vec3& triVel1,
																					 const glm::vec3& triVel2, Collider* meshCollider, long double dt)
{
	TOIResult result;
	result.toi = dt;
	result.colliding = false;
	auto boxBody = boxCollider->getOwnerRigidBody();
	// Mesh body might be static or dynamic
	auto meshBody = meshCollider ? meshCollider->getOwnerRigidBody() : nullptr;
	if (!boxBody)
		return result;
	// --- 1. Get Initial State & Velocities ---
	glm::mat4 boxTransform = *boxBody->transform * glm::translate(glm::mat4(1.0f), boxCollider->getOffset()) *
		glm::mat4_cast(boxCollider->getRotationOffset());
	glm::vec3 boxCenter = glm::vec3(boxTransform[3]);
	const auto* boxData = static_cast<const BoxShapeData*>(boxCollider->getColliderInfo().shapeData.get());
	glm::vec3 boxHalfExtents = boxData->halfExtents;
	std::vector<glm::vec3> boxAxes = zg::components::scenes::PhysicsScene::getBoxWorldAxes(boxCollider);
	glm::vec3 triCenter = (triV0 + triV1 + triV2) / 3.0f;
	glm::vec3 triNormal = glm::normalize(glm::cross(triV1 - triV0, triV2 - triV0));
	if (glm::length2(triNormal) < SAT_EPSILON * SAT_EPSILON)
		return result; // Degenerate triangle
	// Simplified relative velocity: box center vs triangle centroid
	glm::vec3 boxLinVel = boxBody->linearVelocity;
	glm::vec3 triCenterVel = (triVel0 + triVel1 + triVel2) / 3.0f; // Average velocity
	glm::vec3 relativeLinVel = triCenterVel - boxLinVel;
	// --- 2. Swept SAT Test ---
	long double maxEnterTime = 0.0L;
	long double minExitTime = dt;
	glm::vec3 toiNormal = {0, 0, 0};
	std::vector<glm::vec3> testAxes;
	testAxes.reserve(13);
	testAxes.insert(testAxes.end(), boxAxes.begin(), boxAxes.end()); // 3 box axes
	testAxes.push_back(triNormal); // 1 triangle normal
	// 9 cross products (box edge axis x triangle edge)
	glm::vec3 triEdges[] = {triV1 - triV0, triV2 - triV1, triV0 - triV2};
	for (int i = 0; i < 3; ++i)
	{ // Box axes
		for (int j = 0; j < 3; ++j)
		{ // Triangle edges
			if (glm::length2(triEdges[j]) < SAT_EPSILON * SAT_EPSILON)
				continue;
			glm::vec3 crossAxis = glm::cross(boxAxes[i], triEdges[j]);
			if (glm::length2(crossAxis) > SAT_EPSILON * SAT_EPSILON)
			{
				testAxes.push_back(glm::normalize(crossAxis));
			}
		}
	}
	for (const auto& axis : testAxes)
	{
		if (glm::length2(axis) < SAT_EPSILON * SAT_EPSILON)
			continue;
		// Project shapes onto axis at t=0
		float boxMin, boxMax, triMin, triMax;
		projectBox(boxCenter, boxHalfExtents, boxAxes, axis, boxMin, boxMax);
		projectTriangle(triV0, triV1, triV2, axis, triMin, triMax);
		// Project relative velocity onto axis
		float projectedRelVel = glm::dot(relativeLinVel, axis);
		long double tEnterAxis = 0.0L;
		long double tExitAxis = dt;
		if (testAxisSwept(boxMin, boxMax, triMin, triMax, projectedRelVel, dt, tEnterAxis, tExitAxis))
		{
			// Separating axis found
			result.toi = dt;
			result.colliding = false;
			return result;
		}
		// Update overall interval [maxEnterTime, minExitTime]
		if (tEnterAxis > maxEnterTime)
		{
			maxEnterTime = tEnterAxis;
			toiNormal = axis;
		}
		minExitTime = std::min(minExitTime, tExitAxis);
		if (maxEnterTime >= minExitTime - CCD_EPSILON)
		{
			result.toi = dt;
			result.colliding = false;
			return result;
		}
	}
	if (!maxEnterTime)
	{
		result.toi = maxEnterTime;
		result.colliding = false;
		return result;
	}
	// --- 3. Collision Found ---
	result.toi = maxEnterTime;
	result.colliding = true;
	// --- 4. Compute Manifold at TOI ---
	// Ensure normal points from triangle to box (or consistently)
	glm::vec3 directionTriToBox = boxCenter - triCenter; // Direction at t=0
	if (glm::dot(toiNormal, directionTriToBox) < 0.0f)
	{
		toiNormal = -toiNormal;
	}
	// Handle potential zero normal from cross products
	if (glm::length2(toiNormal) < SAT_EPSILON * SAT_EPSILON)
	{
		// Fallback to triangle normal or box-tri direction
		toiNormal = (glm::length2(directionTriToBox) > SAT_EPSILON * SAT_EPSILON) ? glm::normalize(directionTriToBox)
																																							: triNormal; // Or a default like Y-up
	}
	result.manifold.normal = glm::normalize(toiNormal);
	result.manifold.penetrationDepth = 0.0f;
	result.manifold.colliding = true;
	result.manifold.colliderA = boxCollider;
	result.manifold.colliderB = meshCollider; // The collider owning the triangle
	// Placeholder contact point: Project box center onto triangle plane at t=0
	// TODO: Proper contact point calculation at TOI is needed.
	float dist = glm::dot(boxCenter - triV0, result.manifold.normal);
	glm::vec3 contactPoint = boxCenter - dist * result.manifold.normal;
	result.manifold.contactPoints.push_back(contactPoint);
	return result;
}

std::vector<glm::vec3> PhysicsScene::clipPolygonAgainstPlaneInternal(const std::vector<glm::vec3>& polygonVertices,
																																		 const glm::vec3& planeNormal, float planeDistance)
{
	std::vector<glm::vec3> clippedVertices;
	if (polygonVertices.empty())
		return clippedVertices;
	clippedVertices.reserve(polygonVertices.size() + 2);
	for (size_t i = 0; i < polygonVertices.size(); ++i)
	{
		const glm::vec3& p1 = polygonVertices[i];
		const glm::vec3& p2 = polygonVertices[(i + 1) % polygonVertices.size()];
		float dist1 = glm::dot(planeNormal, p1) - planeDistance;
		float dist2 = glm::dot(planeNormal, p2) - planeDistance;
		bool p1_inside = (dist1 <= SAT_EPSILON);
		bool p2_inside = (dist2 <= SAT_EPSILON);
		if (p1_inside && p2_inside)
		{
			clippedVertices.push_back(p2);
		}
		else if (p1_inside && !p2_inside)
		{
			float denom = dist1 - dist2;
			if (std::abs(denom) > SAT_EPSILON)
			{
				float t = dist1 / denom;
				if (t >= 0.0f && t <= 1.0f)
					clippedVertices.push_back(p1 + t * (p2 - p1));
			}
		}
		else if (!p1_inside && p2_inside)
		{
			float denom = dist1 - dist2;
			if (std::abs(denom) > SAT_EPSILON)
			{
				float t = dist1 / denom;
				if (t >= 0.0f && t <= 1.0f)
					clippedVertices.push_back(p1 + t * (p2 - p1));
			}
			clippedVertices.push_back(p2);
		}
	}
	return clippedVertices;
}
bool PhysicsScene::performSATBoxTriangle(Collider* boxCollider, const glm::vec3& triV0, const glm::vec3& triV1,
																				 const glm::vec3& triV2, Collider* meshCollider, CollisionMannifold& manifold)
{
	if (!boxCollider || boxCollider->getShapeType() != ShapeType::Box || !meshCollider)
		return false;
	auto boxBody = boxCollider->getOwnerRigidBody();
	auto meshBody = meshCollider->getOwnerRigidBody();
	if (!boxBody || !meshBody)
		return false;

	std::vector<glm::vec3> boxAxes = getBoxWorldAxesInternal(boxCollider);
	if (boxAxes.size() != 3)
		return false;
	glm::vec3 boxWorldOffset = boxBody->getOrientation() * boxCollider->getOffset();
	glm::vec3 boxCenter = boxBody->getPosition() + boxWorldOffset;

	glm::vec3 triEdge0 = triV1 - triV0;
	glm::vec3 triEdge1 = triV2 - triV1;
	glm::vec3 triEdge2 = triV0 - triV2;
	glm::vec3 triNormal = glm::cross(triEdge0, triEdge1);
	float triNormalLenSq = glm::length2(triNormal);
	if (triNormalLenSq < SAT_EPSILON * SAT_EPSILON)
		return false;
	triNormal /= std::sqrt(triNormalLenSq);

	std::vector<glm::vec3> testAxes;
	testAxes.reserve(13);
	testAxes.insert(testAxes.end(), boxAxes.begin(), boxAxes.end());
	testAxes.push_back(triNormal);
	std::vector<glm::vec3> triEdges = {triEdge0, triEdge1, triEdge2};
	for (const auto& boxAxis : boxAxes)
	{
		for (const auto& triEdge : triEdges)
		{
			glm::vec3 crossAxis = glm::cross(boxAxis, triEdge);
			if (glm::length2(crossAxis) > SAT_EPSILON * SAT_EPSILON)
			{
				testAxes.push_back(glm::normalize(crossAxis));
			}
		}
	}

	float minPenetration = std::numeric_limits<float>::infinity();
	glm::vec3 collisionNormal(0.0f);
	bool separatingAxisFound = false;

	for (const auto& axis : testAxes)
	{
		if (glm::length2(axis) < SAT_EPSILON * SAT_EPSILON)
			continue;
		float boxMin, boxMax, triMin, triMax;
		projectBoxOntoAxisInternal(boxCollider, axis, boxMin, boxMax);
		projectTriangleOntoAxisInternal(triV0, triV1, triV2, axis, triMin, triMax);
		float overlap = glm::min(boxMax, triMax) - glm::max(boxMin, triMin);
		if (overlap < -SAT_EPSILON)
		{
			separatingAxisFound = true;
			break;
		}
		if (overlap < minPenetration)
		{
			minPenetration = overlap;
			collisionNormal = axis;
		}
	}

	if (separatingAxisFound || minPenetration == std::numeric_limits<float>::infinity() || minPenetration < -SAT_EPSILON)
		return false;

	glm::vec3 triCenter = (triV0 + triV1 + triV2) / 3.0f;
	glm::vec3 directionTriToBox = boxCenter - triCenter;
	if (glm::length2(collisionNormal) < SAT_EPSILON * SAT_EPSILON)
	{
		collisionNormal = (glm::length2(directionTriToBox) > SAT_EPSILON * SAT_EPSILON) ? glm::normalize(directionTriToBox)
																																										: glm::vec3(0, 1, 0);
	}
	else
	{
		if (glm::dot(collisionNormal, directionTriToBox) < 0.0f)
		{
			collisionNormal = -collisionNormal;
		}
		float normLen = glm::length(collisionNormal);
		if (normLen > SAT_EPSILON)
		{
			collisionNormal /= normLen;
		}
		else
		{
			collisionNormal = (glm::length2(directionTriToBox) > SAT_EPSILON * SAT_EPSILON)
				? glm::normalize(directionTriToBox)
				: glm::vec3(0, 1, 0);
		}
	}

	manifold.colliding = true;
	manifold.normal = collisionNormal;
	manifold.penetrationDepth = std::max(0.0f, minPenetration);
	manifold.colliderA = boxCollider;
	manifold.colliderB = meshCollider;
	manifold.contactPoints.clear();

	glm::vec3 N = manifold.normal;
	std::vector<glm::vec3> incidentFaceVertices;
	std::vector<glm::vec3> referenceFaceVertices;
	glm::vec3 referenceFaceNormal;
	glm::vec3 referenceFaceCenter;

	float minDotBox = std::numeric_limits<float>::infinity();
	glm::vec3 boxRefNormal;
	for (const auto& axis : boxAxes)
	{
		float dot = glm::dot(N, axis);
		if (dot < minDotBox)
		{
			minDotBox = dot;
			boxRefNormal = axis;
		}
		dot = glm::dot(N, -axis);
		if (dot < minDotBox)
		{
			minDotBox = dot;
			boxRefNormal = -axis;
		}
	}
	float dotTri = glm::dot(N, triNormal);

	if (minDotBox < dotTri + SAT_EPSILON)
	{
		referenceFaceNormal = boxRefNormal;
		referenceFaceVertices = getBoxFaceVerticesInternal(boxCollider, referenceFaceNormal);
		referenceFaceCenter = getBoxFaceCenterInternal(boxCollider, referenceFaceNormal);
		incidentFaceVertices = {triV0, triV1, triV2};
	}
	else
	{
		referenceFaceNormal = triNormal;
		referenceFaceVertices = {triV0, triV1, triV2};
		referenceFaceCenter = triCenter;
		incidentFaceVertices = getBestBoxFaceInternal(boxCollider, N);
	}

	std::vector<glm::vec3> clippedPolygon = incidentFaceVertices;
	size_t numRefVerts = referenceFaceVertices.size();
	for (size_t i = 0; i < numRefVerts; ++i)
	{
		glm::vec3 p1 = referenceFaceVertices[i];
		glm::vec3 p2 = referenceFaceVertices[(i + 1) % numRefVerts];
		glm::vec3 edgeDir = p2 - p1;
		if (glm::length2(edgeDir) < SAT_EPSILON * SAT_EPSILON)
			continue;
		edgeDir = glm::normalize(edgeDir);
		glm::vec3 sidePlaneNormal = glm::cross(edgeDir, referenceFaceNormal);
		if (glm::length2(sidePlaneNormal) < SAT_EPSILON * SAT_EPSILON)
			continue;
		sidePlaneNormal = glm::normalize(sidePlaneNormal);
		float sidePlaneDist = glm::dot(sidePlaneNormal, p1);
		clippedPolygon = clipPolygonAgainstPlaneInternal(clippedPolygon, sidePlaneNormal, sidePlaneDist);
		if (clippedPolygon.empty())
			break;
	}

	float referencePlaneDist = glm::dot(referenceFaceNormal, referenceFaceCenter);
	for (const glm::vec3& incidentPoint : clippedPolygon)
	{
		float distToRefPlane = glm::dot(referenceFaceNormal, incidentPoint) - referencePlaneDist;
		if (distToRefPlane <= SAT_EPSILON)
		{
			glm::vec3 contactPointOnRefPlane = incidentPoint - referenceFaceNormal * distToRefPlane;
			float penetrationCheckDist = glm::dot(manifold.normal, contactPointOnRefPlane - referenceFaceCenter);
			// Keep points that are behind the reference plane but not deeper than penetration depth allows
			if (penetrationCheckDist <= manifold.penetrationDepth + SAT_EPSILON)
			{
				manifold.contactPoints.push_back(contactPointOnRefPlane);
			}
		}
	}
	return true;
}
// void PhysicsScene::resolveCollisionImpulses(double dt)
// {
// 	float fdt = static_cast<float>(dt);
// 	if (fdt <= 1e-9f)
// 	{
// 		return;
// 	}

// 	const float slop = 0.01f;
// 	const float baumgarteBeta = 0.2f;
// 	const int impulseResolutionIterations = 10;
// 	const float restitutionVelocityThreshold = 0.2f; // Don't bounce for very slow collisions

// 	for (int iter = 0; iter < impulseResolutionIterations; ++iter)
// 	{
// 		for (const CollisionMannifold& manifold : collisionContacts)
// 		{
// 			auto colliderA = manifold.colliderA;
// 			auto colliderB = manifold.colliderB;
// 			auto bodyA = colliderA ? colliderA->getOwnerRigidBody() : nullptr;
// 			auto bodyB = colliderB ? colliderB->getOwnerRigidBody() : nullptr;

// 			if (!bodyA || !bodyB || !colliderA || !colliderB)
// 				continue;
// 			if (bodyA->isStatic() && bodyB->isStatic())
// 				continue;
// 			if (colliderA->getIsSensor() || colliderB->getIsSensor())
// 				continue;

// 			float restitution =
// 				std::max(colliderA->getPhysicsMaterial().restitution, colliderB->getPhysicsMaterial().restitution);

// 			glm::mat3 invInertiaWorldA = bodyA->getInverseInertiaTensorWorld();
// 			glm::mat3 invInertiaWorldB = bodyB->getInverseInertiaTensorWorld();

// 			for (const glm::vec3& contactPoint : manifold.contactPoints)
// 			{
// 				glm::vec3 rA = contactPoint - *bodyA->position; // Vector from A's CM to contact point
// 				glm::vec3 rB = contactPoint - *bodyB->position; // Vector from B's CM to contact point

// 				// Relative velocity at the contact point
// 				glm::vec3 velocityA = bodyA->linearVelocity + glm::cross(bodyA->angularVelocity, rA);
// 				glm::vec3 velocityB = bodyB->linearVelocity + glm::cross(bodyB->angularVelocity, rB);
// 				glm::vec3 relativeVelocity = velocityB - velocityA;
// 				float velocityAlongNormal = glm::dot(relativeVelocity, manifold.normal);

// 				// Baumgarte stabilization term (positional correction)
// 				float penetrationError = std::max(0.0f, manifold.penetrationDepth - slop);
// 				float baumgarteBias = (baumgarteBeta / fdt) * penetrationError;

// 				// Calculate effective mass at the contact point (denominator of impulse calculation)
// 				glm::vec3 crossRA_N = glm::cross(rA, manifold.normal);
// 				glm::vec3 crossRB_N = glm::cross(rB, manifold.normal);
// 				float invEffectiveMass = bodyA->inverseMass + bodyB->inverseMass +
// 					glm::dot(invInertiaWorldA * crossRA_N, crossRA_N) + glm::dot(invInertiaWorldB * crossRB_N, crossRB_N);

// 				if (invEffectiveMass <= 1e-9f)
// 				{
// 					continue; // Avoid division by zero if effective mass is infinite
// 				}
// 				float effectiveMass = 1.0f / invEffectiveMass;

// 				// Calculate restitution term (modified desired velocity change)
// 				// Only apply restitution if impact velocity is significant enough
// 				float restitutionTerm = 0.0f;
// 				if (velocityAlongNormal < -restitutionVelocityThreshold)
// 				{
// 					restitutionTerm = -restitution * velocityAlongNormal;
// 				}

// 				// Calculate desired velocity change (including restitution and Baumgarte bias)
// 				// Target velocity = restitution term + positional correction term
// 				// We want to eliminate the current normal velocity (-velocityAlongNormal)
// 				// and add the target velocity.
// 				float desiredDeltaVelocity = -velocityAlongNormal + restitutionTerm + baumgarteBias;


// 				// Calculate the impulse magnitude needed
// 				float impulseMagnitude = effectiveMass * desiredDeltaVelocity;

// 				// Clamp the impulse magnitude (impulses should only push, not pull)
// 				// NOTE: For robust stacking, accumulated impulse clamping is needed.
// 				// This simple version clamps each iteration's impulse independently.
// 				impulseMagnitude = std::max(0.0f, impulseMagnitude);


// 				// Apply the impulse
// 				glm::vec3 impulseVector = manifold.normal * impulseMagnitude;

// 				if (bodyA->isDynamic())
// 				{
// 					bodyA->linearVelocity -= impulseVector * bodyA->inverseMass;
// 					bodyA->angularVelocity -= invInertiaWorldA * glm::cross(rA, impulseVector);
// 				}
// 				if (bodyB->isDynamic())
// 				{
// 					bodyB->linearVelocity += impulseVector * bodyB->inverseMass;
// 					bodyB->angularVelocity += invInertiaWorldB * glm::cross(rB, impulseVector);
// 				}
// 			} // End loop over contact points
// 		} // End loop over manifolds
// 	} // End loop over iterations
// }
void PhysicsScene::resolveCollisionImpulses(double dt)
{
	float fdt = static_cast<float>(dt);

	// Constants - Keep these or tune as needed
	const float slop = 0.01f; // Allowed penetration before correction starts
	const float baumgarteBeta = 0.15f; // Baumgarte stabilization factor (position correction strength)
	// const float velocityCorrectionThreshold = 0.01f; // Unused in original, keep if needed later
	// const int impulseResolutionIterations = 15;    // See note below about iterative solving

	// --- Iterative Solving Loop Placeholder (See Note Below) ---
	// for (int i = 0; i < impulseResolutionIterations; ++i)
	// {
	for (const CollisionMannifold& manifold : collisionContacts)
	{
		auto colliderA = manifold.colliderA;
		auto colliderB = manifold.colliderB;
		auto bodyA = colliderA ? colliderA->getOwnerRigidBody() : nullptr;
		auto bodyB = colliderB ? colliderB->getOwnerRigidBody() : nullptr;

		// Basic validity checks
		if (!bodyA || !bodyB || !colliderA || !colliderB)
			continue;
		if (bodyA->isStatic() && bodyB->isStatic())
			continue;
		if (colliderA->getIsSensor() || colliderB->getIsSensor())
			continue;

		// --- Calculate Collision Properties ---

		// Relative velocity at the point of contact (ignoring angular for now)
		// TODO: If you have angular velocity, include it here:
		// rA = contactPoint - bodyA->position; rB = contactPoint - bodyB->position;
		// relativeVelocity = (bodyB->linearVelocity + cross(bodyB->angularVelocity, rB)) -
		//                    (bodyA->linearVelocity + cross(bodyA->angularVelocity, rA));
		glm::vec3 relativeVelocity = bodyB->linearVelocity - bodyA->linearVelocity;

		// Relative velocity along the normal
		float velocityAlongNormal = glm::dot(relativeVelocity, manifold.normal);

		// Don't resolve if objects are already separating faster than a small threshold
		// (Helps prevent sticking and unnecessary calculations)
		// if (velocityAlongNormal > -0.01f) // Adjust threshold slightly negative if needed
		// {
		// 	// Optional: Add Baumgarte correction even if separating, if penetration exists
		// 	// float penetrationError = std::max(0.0f, manifold.penetrationDepth - slop);
		// 	// if (penetrationError > 0.0f) { /* Apply only bias impulse */ }
		// 	// else { continue; } // If no penetration and separating, nothing to do.
		// 	// For simplicity now, we just continue if separating.
		// 	// However, resting contact often has velocityAlongNormal near zero.
		// 	// A common check is to only apply impulse if velocityAlongNormal < 0
		// 	if (velocityAlongNormal >= 0)
		// 		continue; // Only apply separation/restitution impulse if closing
		// }


		// Combine restitution (common methods: min, max, average)
		// Max ensures bounce if either object is bouncy. Min ensures damping if either is not.
		float restitutionA = colliderA->getPhysicsMaterial().restitution;
		float restitutionB = colliderB->getPhysicsMaterial().restitution;
		float combinedRestitution =
			std::max(restitutionA, restitutionB); // Or std::min, or (restitutionA+restitutionB)*0.5f

		// Calculate effective mass along the normal (ignoring angular inertia for now)
		// TODO: Include inverse inertia tensor if using angular velocity
		float invMassSum = bodyA->inverseMass + bodyB->inverseMass;
		if (invMassSum <= 1e-9f) // Avoid division by zero if both are static/infinite mass
			invMassSum = 1e-9f; // Use a very small number instead of continuing, allows bias impulse for static vs dynamic.
		// continue; // Original behaviour - If you prefer this, uncomment it.

		float effectiveMassNormal = 1.0f / invMassSum;

		// --- Calculate Baumgarte Stabilization (Position Correction) Bias ---
		// float penetrationError = std::max(0.0f, manifold.penetrationDepth - slop);
		// float baumgarteBias = (baumgarteBeta / fdt) * penetrationError;

		// --- Calculate Impulse Magnitude (j) ---
		// The impulse magnitude 'j' needed to achieve the desired separation velocity.
		// Desired separation velocity = -e * closing_velocity + position_correction_velocity
		// j = effectiveMass * (desired_delta_velocity)
		// j = effectiveMass * (desired_separation_velocity - current_velocity_along_normal)
		// j = effectiveMass * ((-combinedRestitution * velocityAlongNormal) + baumgarteBias - velocityAlongNormal)
		// j = effectiveMass * (-(1.0f + combinedRestitution) * velocityAlongNormal + baumgarteBias)

		float j =
			effectiveMassNormal * ((1.0f + combinedRestitution) * velocityAlongNormal /* * manifold.penetrationDepth*/);

		// j = std::max(0.0f, j);

		// --- Apply Impulse ---
		glm::vec3 impulseVec = manifold.normal * j;

		if (bodyA->isDynamic() && bodyB->isDynamic())
		{
			j *= 1;
		}

		if (bodyA->isDynamic())
		{
			bodyA->linearVelocity += impulseVec * bodyA->inverseMass;
		}
		if (bodyB->isDynamic())
		{
			bodyB->linearVelocity -= impulseVec * bodyB->inverseMass;
		}
		continue;
	}
}
void PhysicsScene::synchronizeTransforms() {}
void PhysicsScene::projectBoxOntoAxis(Collider* boxCollider, const glm::vec3& axis, float& minProj, float& maxProj)
{
	// Ensure valid box and associated data/transform
	if (!boxCollider || boxCollider->getShapeType() != ShapeType::Box)
	{
		minProj = maxProj = 0.0f;
		return;
	}
	const auto* boxData = static_cast<const BoxShapeData*>(boxCollider->getColliderInfo().shapeData.get());
	const glm::mat4* worldTransformPtr = boxCollider->getTransform(); // Get from collider's owner RB
	if (!boxData || !worldTransformPtr)
	{
		minProj = maxProj = 0.0f;
		return;
	}

	// Combine world transform with collider's local offset and rotation
	glm::mat4 localOffsetTransform =
		glm::translate(glm::mat4(1.0f), boxCollider->getOffset()) * glm::mat4_cast(boxCollider->getRotationOffset());
	glm::mat4 finalTransform = (*worldTransformPtr) * localOffsetTransform;

	// Box half extents
	const glm::vec3 h = boxData->halfExtents;

	// Calculate the 8 vertices of the box in its local space (relative to collider center)
	glm::vec3 localVertices[8] = {{-h.x, -h.y, -h.z}, {h.x, -h.y, -h.z}, {h.x, h.y, -h.z}, {-h.x, h.y, -h.z},
																{-h.x, -h.y, h.z},	{h.x, -h.y, h.z},	 {h.x, h.y, h.z},	 {-h.x, h.y, h.z}};

	// Initialize min/max projection values
	minProj = std::numeric_limits<float>::max();
	maxProj = std::numeric_limits<float>::lowest(); // Use lowest() for negative infinity equivalent

	// Project each vertex onto the axis
	for (int i = 0; i < 8; ++i)
	{
		// Transform vertex to world space using the combined transform
		glm::vec4 worldVertex4 = finalTransform * glm::vec4(localVertices[i], 1.0f);
		glm::vec3 worldVertex = glm::vec3(worldVertex4); // Convert from vec4

		// Project onto the axis (dot product)
		float projection = glm::dot(worldVertex, axis);

		// Update min and max
		minProj = std::min(minProj, projection);
		maxProj = std::max(maxProj, projection);
	}
}
void PhysicsScene::projectTriangleOntoAxis(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2,
																					 const glm::vec3& axis, float& minProj, float& maxProj)
{
	minProj = std::numeric_limits<float>::max();
	maxProj = std::numeric_limits<float>::lowest();

	float p0 = glm::dot(v0, axis);
	float p1 = glm::dot(v1, axis);
	float p2 = glm::dot(v2, axis);

	minProj = std::min({p0, p1, p2});
	maxProj = std::max({p0, p1, p2});
}
std::vector<glm::vec3> PhysicsScene::getBoxWorldAxes(Collider* boxCollider)
{
	std::vector<glm::vec3> worldAxes;
	worldAxes.reserve(3);

	// Default axes if something is wrong
	auto addDefaultAxes = [&]()
	{
		worldAxes.push_back({1.0f, 0.0f, 0.0f});
		worldAxes.push_back({0.0f, 1.0f, 0.0f});
		worldAxes.push_back({0.0f, 0.0f, 1.0f});
	};

	if (!boxCollider)
	{
		addDefaultAxes();
		return worldAxes;
	}

	const glm::mat4* worldTransformPtr = boxCollider->getTransform(); // From owner RB
	if (!worldTransformPtr)
	{
		addDefaultAxes();
		return worldAxes;
	}

	// Combine world rotation with collider's local rotation offset
	glm::quat worldRot = glm::quat_cast(*worldTransformPtr);
	glm::quat localRot = boxCollider->getRotationOffset();
	glm::quat finalRot = worldRot * localRot; // Combine rotations

	// Convert final rotation quaternion to a 3x3 matrix (or use columns of 4x4)
	glm::mat3 rotationMatrix = glm::mat3_cast(finalRot);

	// Extract axes (columns of the rotation matrix) and normalize
	// Normalization is crucial if the original transform has non-uniform scaling
	worldAxes.push_back(glm::normalize(rotationMatrix[0])); // X-axis
	worldAxes.push_back(glm::normalize(rotationMatrix[1])); // Y-axis
	worldAxes.push_back(glm::normalize(rotationMatrix[2])); // Z-axis

	return worldAxes;
}
