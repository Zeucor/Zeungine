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
	timeAccumulator = 0.0;
	rigidBodies.clear();
	std::cout << "PhysicsScene attached." << std::endl;
}
void PhysicsScene::onUpdate()
{
	timeAccumulator += (float)deltaTime;
	// int subSteps = 0;
	// auto subTimeStep = fixedTimeStep / totalSubSteps;
	// while (subSteps < totalSubSteps && timeAccumulator - subTimeStep > 0)
	// {
	// if (usingCCD)
	stepSimulationCCD(timeAccumulator);
	// else
	// 	stepSimulation(timeAccumulator);
	timeAccumulator -= (float)deltaTime;
	// 	subSteps++;
	// }
	// if (subSteps >= totalSubSteps && timeAccumulator >= fixedTimeStep)
	// {
	// 	std::cerr << "Warning: Physics simulation lagging behind real-time." << std::endl;
	// 	timeAccumulator = std::fmod(timeAccumulator, fixedTimeStep);
	// }
	// synchronizeTransforms();
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
// void PhysicsScene::stepSimulation(float dt)
// {
// 	// 2. Integrate motion (update velocities and predict new positions/rotations)
// 	updateTransforms(dt);
// 	// 3. Collision Detection (find pairs that are colliding and store contacts)
// 	// detectCollisions(); // Fills collisionContacts vector
// 	// --- 4. Resolve Collision Impulses (Velocity Correction using PGS-like approach) ---
// 	// This function now contains the iteration loop internally
// 	resolveCollisionImpulses(dt);
// 	// --- 5. Apply Positional Correction (Position Correction) ---
// 	// Apply positional correction ONCE after the impulse loop is finished
// 	applyPositionalCorrection();
// 	updateVelocities(dt);
// 	// 6. Clear forces for the next step
// 	for (auto body : rigidBodies)
// 	{
// 		if (body)
// 			body->clearForces();
// 	}
// }
void PhysicsScene::stepSimulationCCD(float dt)
{
	if (dt <= 0.0f)
	{
		return;
	}
	clearConstraints();
	for (auto& body : rigidBodies)
	{
		if (body)
		{
			body->clearActiveMannifolds();
		}
	}
	float remainingDt = dt;
	int maxSubSteps = 10;
	int subStepCount = 0;
	if (gravity)
	{
		gravity->applyGravity(*this, dt);
	}
	subStepCount++;
	float subStepDt = remainingDt;
	float earliestTOI = subStepDt;
	std::vector<TOIResult> potentialEvents;
	std::multimap<float, TOIResult> collisionEvents;
	std::vector<std::pair<entities::Collider*, entities::Collider*>> potentialPairs;
	findPotentialCollisionPairs(subStepDt, potentialPairs);
	for (const auto& pair : potentialPairs)
	{
		entities::Collider* colliderA = pair.first;
		entities::Collider* colliderB = pair.second;
		if (!colliderA || !colliderB)
			continue;
		entities::RigidBody* bodyA = colliderA->getOwnerRigidBody();
		entities::RigidBody* bodyB = colliderB->getOwnerRigidBody();
		if (!bodyA || !bodyB)
			continue;
		if (bodyA->getSleeping() && bodyB->getSleeping())
			continue;
		if (bodyA->isStatic() && bodyB->isStatic())
			continue;
		ShapeType typeA = colliderA->getShapeType();
		ShapeType typeB = colliderB->getShapeType();
		TOIResult currentResult;
		currentResult.colliding = false;
		currentResult.toi = subStepDt;
		if (typeA == ShapeType::Box && typeB == ShapeType::Box)
		{
			currentResult = findTOIBoxBox(colliderA, colliderB, subStepDt);
		}
		else if (typeA == ShapeType::Box && typeB == ShapeType::Mesh)
		{
			// currentResult = findTOIBoxMesh(colliderA, colliderB, subStepDt);
		}
		else if (typeA == ShapeType::Mesh && typeB == ShapeType::Box)
		{
			// currentResult = findTOIBoxMesh(colliderB, colliderA, subStepDt);
		}
		if (currentResult.colliding && currentResult.toi >= 0.0f && currentResult.toi <= subStepDt)
		{
			collisionEvents.insert({currentResult.toi, currentResult});
		}
	}
	auto it = collisionEvents.begin();
	float ldt = 0;
	std::unordered_map<RigidBody*, float> bodyUpdatedAtDt;
	if (it != collisionEvents.end())
	{
		while (it != collisionEvents.end())
		{
			float currentKey = it->first;
			float idt = currentKey - ldt;
			ldt = currentKey;
			std::vector<CollisionMannifold> valuesForKey;
			auto rangeEndIt = collisionEvents.upper_bound(currentKey);
			for (auto currentIt = it; currentIt != rangeEndIt; ++currentIt)
			{
				collisionContacts.push_back(currentIt->second.manifold);
				resolveConstraint(currentIt->second.manifold);
				auto bodyA = currentIt->second.manifold.colliderA->getOwnerRigidBody();
				auto bodyB = currentIt->second.manifold.colliderB->getOwnerRigidBody();
				//
				bodyA->addActiveMannifold(currentIt->second.manifold);
				bodyB->addActiveMannifold(currentIt->second.manifold);
				//
				auto& bodyAUpdatedDt = bodyUpdatedAtDt[bodyA];
				auto bAdt = ldt - bodyAUpdatedDt;
				bodyA->update(bAdt, false);
				bodyAUpdatedDt = ldt;
				//
				auto& bodyBUpdatedDt = bodyUpdatedAtDt[bodyB];
				auto bBdt = ldt - bodyBUpdatedDt;
				bodyB->update(bBdt, false);
				bodyBUpdatedDt = ldt;
				//
				resolveCollisionImpulses(1);
				collisionContacts.clear();
				//
				// applyConstraints(bodyA);
				// applyConstraints(bodyB);
			}
			remainingDt = dt - ldt;
			it = rangeEndIt;
		}
	}
	for (auto& body : rigidBodies)
	{
		auto& bodyUpdatedDt = bodyUpdatedAtDt[body];
		auto bdt = dt - bodyUpdatedDt;
		body->update(bdt);
	}
	for (auto& body : rigidBodies)
	{
		if (body)
		{
			applyConstraints(body);
		}
	}
}

void PhysicsScene::applyConstraints(RigidBody* body)
{
	auto rbIter = colliderConstraints.find(body);
	if (rbIter != colliderConstraints.end())
	{
		for (auto& cs : rbIter->second)
		{
			cs.apply();
		}
		auto& rbInfo = rbIter->first->getInfo();
		rbInfo.entity.updateNonce--;
		rbInfo.entity.getModelMatrix();
		for (auto& collider : rbIter->first->getColliders())
		{
			collider->getWorldAABB();
		}
	}
}

void PhysicsScene::resolveConstraint(const CollisionMannifold& collisionMannifold)
{
	// 1. Determine the primary axis of collision.
	// This simple version assumes axis-aligned collisions for this constraint type.
	// A more robust system might handle arbitrary normals differently.
	int axis = -1;
	float max_normal_comp = 0.f; // Find axis with largest normal component
	for (int i = 0; i < 3; i++)
	{
		float abs_comp = std::abs(collisionMannifold.normal[i]);
		// Use a threshold slightly less than 1 to account for floating point inaccuracies
		// if (abs_comp > 0.99f) {
		//     axis = i;
		//     break; // Found a clear axis-aligned collision
		// }
		if (abs_comp > max_normal_comp)
		{
			max_normal_comp = abs_comp;
			axis = i;
		}
	}

	// Only proceed if a dominant axis is found (avoids issues with grazing collisions if only handling axis-aligned)
	// Adjust the threshold (e.g., 0.7f) depending on how strictly axis-aligned you need it.
	if (axis == -1 || max_normal_comp < 0.7f)
	{
		return; // Ignore collisions not closely aligned with a world axis for this constraint type
	}


	// 2. Get pointers to the rigid bodies and their colliders
	auto* bodyA = collisionMannifold.colliderA->getOwnerRigidBody();
	auto* bodyB = collisionMannifold.colliderB->getOwnerRigidBody();

	// Ensure bodies exist
	if (!bodyA || !bodyB)
		return;

	// 3. Get references to world and local AABBs
	// NOTE: getWorldAABB() must provide access to the *current*, up-to-date world AABB
	AABB& worldAABB_A = (AABB&)collisionMannifold.colliderA->getWorldAABB();
	AABB& worldAABB_B = (AABB&)collisionMannifold.colliderB->getWorldAABB();

	auto localHalfExtentsA = collisionMannifold.colliderA->getColliderInfo().shapeData->getHalfExtents();
	auto localHalfExtentsB = collisionMannifold.colliderB->getColliderInfo().shapeData->getHalfExtents();

	// 4. Determine constraint type (Normal) and identify relevant AABB boundaries based on the collision normal direction
	Constraint::Normal normalA, normalB;
	float* refA = nullptr; // Pointer to the relevant float boundary value in worldAABB_A (_min[axis] or _max[axis])
	float* refB = nullptr; // Pointer to the relevant float boundary value in worldAABB_B (_min[axis] or _max[axis])

	// Use the sign of the normal component along the dominant axis
	if (collisionMannifold.normal[axis] <
			0) // Normal points from B towards A along negative axis (e.g., A is below B on Y axis)
	{
		// Body A's min face is constrained by Body B's max face
		normalA = Constraint::AtOrAbove; // A's min[axis] must be >= B's max[axis]
		normalB = Constraint::AtOrBelow; // B's max[axis] must be <= A's min[axis]
		refA = &(worldAABB_A._min[axis]); // Reference A's minimum boundary on this axis
		refB = &(worldAABB_B._max[axis]); // Reference B's maximum boundary on this axis
	}
	else // Normal points from B towards A along positive axis (e.g., A is above B on Y axis)
			 // (collisionMannifold.normal[axis] > 0)
	{
		// Body A's max face is constrained by Body B's min face
		normalA = Constraint::AtOrBelow; // A's max[axis] must be <= B's min[axis]
		normalB = Constraint::AtOrAbove; // B's min[axis] must be >= A's max[axis]
		refA = &(worldAABB_A._max[axis]); // Reference A's maximum boundary on this axis
		refB = &(worldAABB_B._min[axis]); // Reference B's minimum boundary on this axis
	}

	// Check if references were assigned (should always happen if axis != -1)
	if (!refA || !refB)
	{
		// Handle error or log warning - shouldn't happen with the axis check above
		return;
	}

	// 5. Get references/pointers for position and rotation
	// IMPORTANT: getPosition() MUST return glm::vec3& (a non-const reference) for modification to work.
	// If it returns by value or const&, the constraint cannot modify the position directly.
	glm::vec3& posVecA = (glm::vec3&)bodyA->getPosition();
	glm::vec3& posVecB = (glm::vec3&)bodyB->getPosition();
	float* posA = &(posVecA[axis]); // Pointer to the specific position component of A
	float* posB = &(posVecB[axis]); // Pointer to the specific position component of B

	// Get const references to rotations (assuming getOrientation returns const glm::quat&)
	const glm::quat& rotA = bodyA->getOrientation();
	const glm::quat& rotB = bodyB->getOrientation();


	// 6. Add constraints to the map, but only for dynamic bodies
	// The constraint for body A uses B's boundary as 'other'
	if (bodyA->isDynamic())
	{
		// Pass references to A's boundary, B's boundary, A's position component, A's rotation, and A's local size
		colliderConstraints[bodyA].emplace_back(normalA, axis, *refA, *refB, *posA, rotA, localHalfExtentsA);
	}
	// The constraint for body B uses A's boundary as 'other'
	if (bodyB->isDynamic())
	{
		// Pass references to B's boundary, A's boundary, B's position component, B's rotation, and B's local size
		colliderConstraints[bodyB].emplace_back(normalB, axis, *refB, *refA, *posB, rotB, localHalfExtentsB);
	}
}

void PhysicsScene::applyContstraints() {}

void PhysicsScene::clearConstraints() { colliderConstraints.clear(); }

void PhysicsScene::findPotentialCollisionPairs(
	float dt, std::vector<std::pair<entities::Collider*, entities::Collider*>>& potentialPairs)
{
	potentialPairs.clear();
	std::vector<Collider*> candidates;
	// Use a set to store unique pairs, ensuring order (e.g., lower pointer first)
	std::unordered_set<std::pair<Collider*, Collider*>> uniquePairs;
	for (auto bodyAIter = rigidBodies.begin(), rigidBodiesEnd = rigidBodies.end(); bodyAIter != rigidBodiesEnd;
			 ++bodyAIter)
	{
		auto& bodyA = *bodyAIter;
		const auto& collidersA = bodyA->getColliders();
		for (auto colliderA : collidersA)
		{
			if (!colliderA)
				continue;
			candidates.clear();
			AABB colliderAAABB = colliderA->getSweptWorldAABB(dt);
			for (auto bodyBIter = bodyAIter + 1; bodyBIter != rigidBodiesEnd; ++bodyBIter)
			{
				auto bodyB = *bodyBIter;
				const auto& collidersB = bodyB->getColliders();
				for (auto colliderB : collidersB)
				{
					if (!colliderA)
						continue;
					AABB colliderBAABB = colliderB->getSweptWorldAABB(dt);
					auto overlaps = colliderAAABB.overlaps(colliderBAABB);
					if (overlaps != AABB::Overlaps::None)
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

void PhysicsScene::findPotentialCollisionPairs(
	RigidBody& body, float dt, std::vector<std::pair<entities::Collider*, entities::Collider*>>& potentialPairs)
{
	potentialPairs.clear();
	std::vector<Collider*> candidates;
	std::unordered_set<std::pair<Collider*, Collider*>> uniquePairs;
	const auto& collidersA = body.getColliders();
	for (auto colliderA : collidersA)
	{
		if (!colliderA)
			continue;
		candidates.clear();
		AABB colliderAAABB = colliderA->getSweptWorldAABB(dt);
		for (auto bodyBIter = rigidBodies.begin(), rigidBodiesEnd = rigidBodies.end(); bodyBIter != rigidBodiesEnd;
				 ++bodyBIter)
		{
			auto& bodyB = *bodyBIter;
			if (bodyB == &body)
			{
				continue;
			}
			const auto& collidersB = bodyB->getColliders();
			for (auto colliderB : collidersB)
			{
				if (!colliderA)
					continue;
				AABB colliderBAABB = colliderB->getSweptWorldAABB(dt);
				auto overlaps = colliderAAABB.overlaps(colliderBAABB);
				if (overlaps != AABB::Overlaps::None)
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

		// if (usingCCD)
		// {
		// 	assert(false);
		// }

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
			correctionVector = manifold.normal * manifold.penetrationDepth;
		}
		else
		{
			correctionVector = manifold.normal * manifold.penetrationDepth;
		}

		// Apply the correction
		if (bodyA->isDynamic())
		{
			bodyA->translate(correctionVector * bodyA->getInverseMass());
		}
		if (bodyB->isDynamic())
		{
			bodyB->translate(-correctionVector * bodyB->getInverseMass());
		}
		continue;
	}
	// Removed return value as it wasn't used in the loop structure anymore
}
void PhysicsScene::updateTransforms(float dt) {}
void PhysicsScene::updateVelocities(float dt) {}
bool PhysicsScene::performSATBoxBox(Collider* boxA, Collider* boxB, CollisionMannifold& manifold)
{
	// // --- Input Validation ---
	// if (!boxA || boxA->getShapeType() != ShapeType::Box || !boxB || boxB->getShapeType() != ShapeType::Box)
	// {
	// 	return false; // Invalid input
	// }
	// auto bodyA = boxA->getOwnerRigidBody();
	// auto bodyB = boxB->getOwnerRigidBody();
	// if (!bodyA || !bodyB)
	// {
	// 	// Need rigid bodies to get transforms and positions
	// 	std::cerr << "Warning: SATBoxBox requires colliders to have RigidBody owners." << std::endl;
	// 	return false;
	// }


	// // --- SAT Initialization ---
	// float minPenetration = std::numeric_limits<float>::infinity();
	// glm::vec3 collisionNormal(0.0f);
	// bool separatingAxisFound = false; // Flag to track if we found *any* separating axis

	// // --- Get Axes to Test ---
	// // Get world-aligned axes for both boxes (3 each)
	// std::vector<glm::vec3> axesA = getBoxWorldAxes(boxA);
	// std::vector<glm::vec3> axesB = getBoxWorldAxes(boxB);

	// // Combine axes: 3 from A, 3 from B, up to 9 from cross products
	// std::vector<glm::vec3> testAxes;
	// testAxes.reserve(15); // 3 + 3 + 9 = 15 potential axes
	// testAxes.insert(testAxes.end(), axesA.begin(), axesA.end());
	// testAxes.insert(testAxes.end(), axesB.begin(), axesB.end());

	// // Generate cross product axes (edge-edge directions)
	// for (const auto& axisA : axesA)
	// {
	// 	for (const auto& axisB : axesB)
	// 	{
	// 		glm::vec3 crossAxis = glm::cross(axisA, axisB);
	// 		// Check for non-zero length (handles parallel axes) using squared length for efficiency
	// 		if (glm::length2(crossAxis) > 1e-8f)
	// 		{
	// 			testAxes.push_back(glm::normalize(crossAxis));
	// 		}
	// 	}
	// }

	// // --- Test Projections on Each Axis ---
	// for (const auto& axis : testAxes)
	// {
	// 	// Check for degenerate axes (should be rare after normalization, but safe)
	// 	if (glm::length2(axis) < 1e-8f)
	// 		continue;

	// 	float minA, maxA, minB, maxB;
	// 	projectBoxOntoAxis(boxA, axis, minA, maxA);
	// 	projectBoxOntoAxis(boxB, axis, minB, maxB);

	// 	// Calculate overlap on this axis
	// 	// overlap = min(maxA, maxB) - max(minA, minB)
	// 	float currentOverlap = glm::min(maxA, maxB) - glm::max(minA, minB);

	// 	// --- Check for Separation ---
	// 	if (currentOverlap < 0.0f)
	// 	{
	// 		// Found a separating axis! No collision possible.
	// 		separatingAxisFound = true;
	// 		break; // Exit the loop early
	// 	}

	// 	// --- Update Minimum Penetration ---
	// 	// **CRITICAL CHANGE:** Update penetration based on *any* axis that shows overlap.
	// 	// The axis with the *smallest* overlap corresponds to the minimum penetration depth.
	// 	if (currentOverlap < minPenetration)
	// 	{
	// 		minPenetration = currentOverlap;
	// 		collisionNormal = axis; // Store the axis associated with this minimum overlap
	// 	}
	// }

	// // --- Final Result ---
	// if (separatingAxisFound)
	// {
	// 	return false; // No collision because a separating axis was found
	// }

	// // If no separating axis was found after checking all axes, the boxes are colliding.
	// // We should have a valid collisionNormal and minPenetration by now.

	// // --- Finalize Manifold ---
	// // Ensure the normal points from B to A (or consistently, e.g., A to B)
	// // Get approximate centers using RigidBody position + Collider offset
	// glm::vec3 centerA = bodyA->getPosition() + (bodyA->getOrientation() * boxA->getOffset()); // Apply rotation to
	// offset glm::vec3 centerB = bodyB->getPosition() + (bodyB->getOrientation() * boxB->getOffset()); // Apply rotation
	// to offset glm::vec3 directionBA = centerA - centerB; // Vector from B's center to A's center

	// // Flip the normal if it's pointing in the "wrong" direction relative to the centers
	// if (glm::dot(collisionNormal, directionBA) < 0.0f)
	// {
	// 	collisionNormal = -collisionNormal; // Ensure normal points roughly from B towards A
	// }

	// manifold.colliding = true;
	// manifold.normal = glm::normalize(collisionNormal); // Ensure normal is unit length
	// manifold.penetrationDepth = minPenetration;
	// // manifold.colliderA = boxA; // Already set in detectCollisions
	// // manifold.colliderB = boxB; // Already set in detectCollisions

	// // TODO: Calculate Contact Points (Advanced)
	// // This often involves finding the features (vertices, edges, faces) that correspond
	// // to the minimum penetration axis and then clipping them against each other.
	// // For box-box, common methods include Sutherland-Hodgman clipping or finding
	// // the intersection of the relevant features.
	// // As a placeholder, you could use the point on boxA closest to boxB along the normal,
	// // or the midpoint of the overlap interval projected back onto the boxes.
	// // Example placeholder: Contact point approx center of overlap
	// // glm::vec3 contactPoint = centerB + collisionNormal * (glm::length(centerA - centerB) - minPenetration * 0.5f);
	// // manifold.contactPoints.push_back(contactPoint);

	return true; // Collision detected
}

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

/**
 * @brief Calculates the world transform matrix of a collider at a future time t.
 * Assumes constant linear and angular velocity over the interval [0, t].
 * @param collider The collider to calculate the transform for.
 * @param t The time offset from the beginning of the frame.
 * @return The predicted 4x4 world transform matrix at time t.
 */
std::pair<glm::vec3, glm::quat> PhysicsScene::getTransformsAtTime(RigidBody* rb, float t, bool updateVelocities)
{
	auto currentPos = rb->getPosition();
	auto linearAcceleration = rb->getForceAccumulator() * rb->getInverseMass();
	auto linearVelocity = rb->getLinearVelocity();
	linearVelocity = linearVelocity + linearAcceleration * t;
	if (updateVelocities)
		rb->setLinearVelocity(linearVelocity);
	auto finalPos = currentPos + linearVelocity * t;

	auto torque = rb->getTorqueAccumulator();
	auto inverseInertiaTensor = rb->getInverseInertiaTensorWorld();
	auto angularAcceleration = inverseInertiaTensor * torque;
	auto angularVelocity = rb->getAngularVelocity();
	angularVelocity = angularVelocity + angularAcceleration * t;
	if (updateVelocities)
		rb->setAngularVelocity(angularVelocity);
	auto finalOrientation = rb->getOrientation();
	float angle = glm::length(angularVelocity) * t;
	if (angle > 0)
	{
		glm::vec3 axis = glm::normalize(angularVelocity);
		glm::quat deltaRotation = glm::angleAxis(angle, axis);
		finalOrientation = glm::normalize(deltaRotation * finalOrientation);
	}
	return {finalPos, finalOrientation};
}

/**
 * @brief Projects an OBB onto a given axis.
 * @param obb The Oriented Bounding Box to project.
 * @param axis The normalized axis to project onto.
 * @return A Projection struct containing the min and max extent of the OBB along the axis.
 */
Projection projectOBB(const OBB& obb, const glm::vec3& axis)
{
	Projection p;
	float centerProjection = glm::dot(obb.center, axis);
	float radiusProjection = obb.halfExtents.x * std::abs(glm::dot(obb.axes[0], axis)) +
		obb.halfExtents.y * std::abs(glm::dot(obb.axes[1], axis)) +
		obb.halfExtents.z * std::abs(glm::dot(obb.axes[2], axis));
	p.min = centerProjection - radiusProjection;
	p.max = centerProjection + radiusProjection;
	return p;
}

// --- Clipping Helper Function ---
// Clips a polygon (list of vertices) against a single plane.
// Returns the vertices of the clipped polygon.
// Vertices are assumed to be ordered (e.g., clockwise or counter-clockwise).
// std::vector<glm::vec3> clipPolygonAgainstPlane(const std::vector<glm::vec3>& vertices,
// 																							 const Plane& clipPlane)
// {
// 	std::vector<glm::vec3> clippedVertices;
// 	if (vertices.empty())
// 	{
// 		return clippedVertices;
// 	}
// 	clippedVertices.reserve(vertices.size() + 2); // Reserve space, might grow slightly

// 	glm::vec3 startPoint = vertices.back();
// 	float startDist = clipPlane.signedDistance(startPoint);

// 	for (const auto& endPoint : vertices)
// 	{
// 		float endDist = clipPlane.signedDistance(endPoint);

// 		bool startInside = startDist >= 0.0f; // Treat points on the plane as inside
// 		bool endInside = endDist >= 0.0f;

// 		if (startInside && endInside)
// 		{
// 			// Edge fully inside, add end point
// 			clippedVertices.push_back(endPoint);
// 		}
// 		else if (startInside && !endInside)
// 		{
// 			// Edge crosses from inside to outside, add intersection
// 			float t = startDist / (startDist - endDist); // Interpolation factor
// 			if (t < 0.0f)
// 				t = 0.0f; // Clamp to avoid issues with parallel lines slightly off
// 			if (t > 1.0f)
// 				t = 1.0f;
// 			clippedVertices.push_back(startPoint + (endPoint - startPoint) * t);
// 		}
// 		else if (!startInside && endInside)
// 		{
// 			// Edge crosses from outside to inside, add intersection and end point
// 			float t = startDist / (startDist - endDist);
// 			if (t < 0.0f)
// 				t = 0.0f;
// 			if (t > 1.0f)
// 				t = 1.0f;
// 			clippedVertices.push_back(startPoint + (endPoint - startPoint) * t);
// 			clippedVertices.push_back(endPoint);
// 		}
// 		// Else (both outside), add nothing

// 		startPoint = endPoint;
// 		startDist = endDist;
// 	}

// 	return clippedVertices;
// }

namespace std
{
	template <>
	struct hash<glm::vec3>
	{
		size_t operator()(const glm::vec3& v) const
		{
			// Simple hash combining components - consider a more robust hash function if needed
			size_t h1 = hash<float>()(v.x);
			size_t h2 = hash<float>()(v.y);
			size_t h3 = hash<float>()(v.z);
			// Combine hashes (boost::hash_combine style)
			size_t seed = 0;
			seed ^= h1 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			return seed;
		}
	};
	// Define approx equality for vec3 needed for unordered_set with spatial hashing
	bool operator==(const glm::vec3& lhs, const glm::vec3& rhs)
	{
		return glm::all(glm::lessThanEqual(glm::abs(lhs - rhs), glm::vec3(0.f)));
	}
} // namespace std

struct ContactPointInfo
{
	glm::vec3 point;
	float penetration; // Penetration depth associated with this point

	bool operator<(const ContactPointInfo& other) const
	{
		// Sort in descending order of penetration (deepest first)
		return penetration > other.penetration;
	}
};

// Clips a polygon (represented by vertices) against a single plane.
// Returns the vertices of the clipped polygon.
std::vector<glm::vec3> clipPolygonAgainstPlane(const std::vector<glm::vec3>& polygon, const Plane& plane)
{
	std::vector<glm::vec3> clippedPolygon;
	if (polygon.empty())
		return clippedPolygon;
	clippedPolygon.reserve(polygon.size() + 1);

	for (size_t i = 0; i < polygon.size(); ++i)
	{
		const glm::vec3& currentVertex = polygon[i];
		const glm::vec3& nextVertex = polygon[(i + 1) % polygon.size()];

		float distCurrent = plane.distanceToPoint(currentVertex);
		float distNext = plane.distanceToPoint(nextVertex);

		bool currentInside = distCurrent >= 0;
		bool nextInside = distNext >= 0;

		if (currentInside && nextInside)
		{
			clippedPolygon.push_back(nextVertex);
		}
		else if (currentInside && !nextInside)
		{
			if (std::abs(distCurrent - distNext) > 0)
			{
				float t = distCurrent / (distCurrent - distNext);
				glm::vec3 intersection = currentVertex + t * (nextVertex - currentVertex);
				clippedPolygon.push_back(intersection);
			}
			else
			{
				if (std::abs(distCurrent) == 0)
					clippedPolygon.push_back(currentVertex);
			}
		}
		else if (!currentInside && nextInside)
		{
			if (std::abs(distCurrent - distNext) > 0)
			{
				float t = distCurrent / (distCurrent - distNext);
				glm::vec3 intersection = currentVertex + t * (nextVertex - currentVertex);
				clippedPolygon.push_back(intersection);
			}
			clippedPolygon.push_back(nextVertex);
		}
	}
	return clippedPolygon;
}


// Clips a polygon against a set of planes (representing the sides of a face/volume)
std::vector<glm::vec3> clipPolygonAgainstPlanes(std::vector<glm::vec3> polygon, const std::vector<Plane>& planes)
{
	for (const auto& plane : planes)
	{
		polygon = clipPolygonAgainstPlane(polygon, plane);
		if (polygon.empty())
			break; // Early exit if clipped away entirely
	}
	return polygon;
}


// Calculates the closest points between two line segments
float closestPointSegmentSegment(const glm::vec3& p1, const glm::vec3& q1, const glm::vec3& p2, const glm::vec3& q2,
																 float& s, float& t, glm::vec3& c1, glm::vec3& c2)
{
	glm::vec3 d1 = q1 - p1;
	glm::vec3 d2 = q2 - p2;
	glm::vec3 r = p1 - p2;
	float a = glm::dot(d1, d1);
	float e = glm::dot(d2, d2);
	float f = glm::dot(d2, r);

	if (a <= 0 && e <= 0)
	{
		s = t = 0.0f;
		c1 = p1;
		c2 = p2;
		return glm::dot(c1 - c2, c1 - c2);
	}
	if (a <= 0)
	{
		s = 0.0f;
		t = glm::clamp(f / e, 0.0f, 1.0f);
	}
	else
	{
		float c = glm::dot(d1, r);
		if (e <= 0)
		{
			t = 0.0f;
			s = glm::clamp(-c / a, 0.0f, 1.0f);
		}
		else
		{
			float b = glm::dot(d1, d2);
			float denom = a * e - b * b;
			if (denom > 0)
			{
				s = glm::clamp((b * f - c * e) / denom, 0.0f, 1.0f);
			}
			else
			{
				s = 0.0f;
			}
			t = glm::dot(p1 + d1 * s - p2, d2);
			if (t < 0.0f)
			{
				t = 0.0f;
				s = glm::clamp(-c / a, 0.0f, 1.0f);
			}
			else if (t > e)
			{
				t = 1.0f;
				s = glm::clamp((b - c) / a, 0.0f, 1.0f);
			}
			else
			{
				if (e > 0)
				{
					t /= e;
				}
				else
				{
					t = 0.0f;
				}
			}
		}
	}
	c1 = p1 + d1 * s;
	c2 = p2 + d2 * t;
	return glm::dot(c1 - c2, c1 - c2);
}

// Helper function to find the specific edge on an OBB most parallel to a direction 'edgeDir'
// and closest to a point 'refPoint' (typically the center of the other OBB).
// This is still complex to get exactly right. A simpler heuristic might be needed.
// This function attempts to find the edge based on maximizing dot products.
std::pair<glm::vec3, glm::vec3> findClosestEdge(const OBB& obb, const glm::vec3& edgeDir, const glm::vec3& refPoint)
{
	float minDistSq = std::numeric_limits<float>::max();
	std::pair<glm::vec3, glm::vec3> bestEdge;
	bestEdge.first = obb.center;
	bestEdge.second = obb.center;

	int parallelAxis = -1;
	float maxAxisDot = -1.0f;
	for (int i = 0; i < 3; ++i)
	{
		float dotVal = std::abs(glm::dot(obb.axes[i], edgeDir));
		if (dotVal > maxAxisDot)
		{
			maxAxisDot = dotVal;
			parallelAxis = i;
		}
	}

	if (parallelAxis == -1)
		return bestEdge;

	int perpAxis1 = (parallelAxis + 1) % 3;
	int perpAxis2 = (parallelAxis + 2) % 3;

	for (float sign1 = -1.0f; sign1 <= 1.0f; sign1 += 2.0f)
	{
		for (float sign2 = -1.0f; sign2 <= 1.0f; sign2 += 2.0f)
		{
			std::pair<glm::vec3, glm::vec3> currentEdge =
				obb.getEdgeVertices(parallelAxis, perpAxis1, perpAxis2, sign1, sign2);
			glm::vec3 midPoint = (currentEdge.first + currentEdge.second) * 0.5f;
			float distSq = glm::length2(refPoint - midPoint);
			if (distSq < minDistSq)
			{
				minDistSq = distSq;
				bestEdge = currentEdge;
			}
		}
	}
	return bestEdge;
}

// --- Main Intersection Function ---
bool staticOBBIntersection(const OBB& obbA, const OBB& obbB, CollisionMannifold& outManifold)
{
	float minPenetration = std::numeric_limits<float>::max();
	glm::vec3 collisionNormal = {0.0f, 0.0f, 0.0f};
	bool foundCollisionAxis = false;
	int bestAxisIndex = -1; // 0-2: A's axes, 3-5: B's axes, 6-14: Cross products
	int faceAxisA_idx = -1, faceAxisB_idx = -1; // Store indices of axes involved in cross product
	glm::vec3 axesToTest[15];
	int axisCount = 0;
	// OBB A's face normals
	axesToTest[axisCount++] = obbA.axes[0];
	axesToTest[axisCount++] = obbA.axes[1];
	axesToTest[axisCount++] = obbA.axes[2];
	// OBB B's face normals
	axesToTest[axisCount++] = obbB.axes[0];
	axesToTest[axisCount++] = obbB.axes[1];
	axesToTest[axisCount++] = obbB.axes[2];
	// Edge-Edge cross products
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			glm::vec3 crossProduct = glm::cross(obbA.axes[i], obbB.axes[j]);
			float lenSq = glm::length2(crossProduct);
			if (lenSq > 0)
			{
				if (axisCount < 15)
				{
					axesToTest[axisCount++] = crossProduct / std::sqrt(lenSq); // Normalize
				}
			}
		}
	}
	// --- Separating Axis Theorem (SAT) ---
	glm::vec3 centerDiff = obbB.center - obbA.center;
	float centerDistSq = glm::length2(centerDiff);
	for (int i = 0; i < axisCount; ++i)
	{
		glm::vec3 currentAxis = axesToTest[i];
		float radiusA = obbA.projectedRadius(currentAxis);
		float radiusB = obbB.projectedRadius(currentAxis);
		float distCentersProj = std::abs(glm::dot(centerDiff, currentAxis));
		float separation = distCentersProj - (radiusA + radiusB);
		if (separation > 0)
		{
			outManifold.colliding = false;
			outManifold.contactCount = 0;
			outManifold.contactPoints.clear();
			return false; // Found separating axis
		}
		float penetration = (radiusA + radiusB) - distCentersProj;
		// Use axis that provides minimum penetration. Add tolerance for near-equal penetrations?
		if (penetration < minPenetration)
		{ // Using strict less than might be better
			minPenetration = penetration;
			collisionNormal = currentAxis;
			bestAxisIndex = i;
			foundCollisionAxis = true;
		}
	}

	// If SAT passes, there is overlap.
	if (!foundCollisionAxis)
	{
		// Should not happen if SAT is correct and axes are valid.
		// Handle potential full containment or degenerate cases.
		if (centerDistSq <= 0)
		{
			collisionNormal = glm::vec3(0.0f, 1.0f, 0.0f);
			minPenetration = obbA.halfExtents.y + obbB.halfExtents.y;
			bestAxisIndex = 1; // Assume face contact
		}
		else
		{
			collisionNormal = glm::normalize(centerDiff);
			float radiusA = obbA.projectedRadius(collisionNormal);
			float radiusB = obbB.projectedRadius(collisionNormal);
			minPenetration = radiusA + radiusB - glm::dot(centerDiff, collisionNormal);
			bestAxisIndex = 0; // Assume face contact
		}
		minPenetration = std::max(0.0f, minPenetration);
		foundCollisionAxis = true;
	}

	// Ensure the collision normal points from A to B
	if (glm::dot(centerDiff, collisionNormal) < 0.0f)
	{
		collisionNormal = -collisionNormal;
	}
	// Final normalization check
	float normLenSq = glm::length2(collisionNormal);
	if (normLenSq < 0.99f || normLenSq > 1.01f)
	{
		if (normLenSq > 0)
		{
			collisionNormal = glm::normalize(collisionNormal);
		}
		else
		{
			collisionNormal = (centerDistSq > 0) ? glm::normalize(centerDiff) : glm::vec3(0.0f, 1.0f, 0.0f);
		}
	}

	// --- Collision Confirmed - Generate Contact Points ---
	outManifold.colliding = true;
	outManifold.normal = collisionNormal;
	// Ensure penetration depth isn't negative due to float errors
	outManifold.penetrationDepth = std::max(0.0f, minPenetration);
	outManifold.contactPoints.clear();
	outManifold.contactCount = 0;

	std::vector<ContactPointInfo> potentialContacts;

	// --- Contact Point Generation ---

	// Case 1: Edge-Edge Contact (SAT axis was a cross product)
	if (bestAxisIndex >= 6)
	{
		int crossIdx = bestAxisIndex - 6;
		faceAxisA_idx = crossIdx / 3;
		faceAxisB_idx = crossIdx % 3;
		glm::vec3 edgeDirA = obbA.axes[faceAxisA_idx];
		glm::vec3 edgeDirB = obbB.axes[faceAxisB_idx];

		std::pair<glm::vec3, glm::vec3> edgeA = findClosestEdge(obbA, edgeDirA, obbB.center);
		std::pair<glm::vec3, glm::vec3> edgeB = findClosestEdge(obbB, edgeDirB, obbA.center);

		float s, t;
		glm::vec3 c1, c2; // c1 on edgeA, c2 on edgeB
		closestPointSegmentSegment(edgeA.first, edgeA.second, edgeB.first, edgeB.second, s, t, c1, c2);

		// Contact point is midpoint between closest points
		glm::vec3 contactPoint = (c1 + c2) * 0.5f;
		// Penetration is distance between c1 and c2 projected onto normal? Or just use manifold depth?
		// Let's use the overall penetration depth for consistency in edge case
		float pointPenetration = outManifold.penetrationDepth;

		// Store the point on A (c1) and its penetration depth
		potentialContacts.push_back({c1, pointPenetration});
	}
	// Case 2: Face-Based Contact (SAT axis was a face normal)
	else
	{
		// Determine Reference and Incident objects/faces based on normal
		const OBB* refOBB = nullptr;
		const OBB* incOBB = nullptr;
		int refAxisIdx = -1;
		bool refPositiveFace = false;
		int incAxisIdx = -1;
		bool incPositiveFace = false;

		// Find face on A most opposing normal (-collisionNormal)
		int tempAxisA;
		bool tempPosA;
		float dotA = -std::numeric_limits<float>::max();
		for (int i = 0; i < 3; ++i)
		{
			float d = glm::dot(obbA.axes[i], -collisionNormal);
			if (d > dotA)
			{
				dotA = d;
				tempAxisA = i;
				tempPosA = false;
			}
			d = glm::dot(-obbA.axes[i], -collisionNormal);
			if (d > dotA)
			{
				dotA = d;
				tempAxisA = i;
				tempPosA = true;
			}
		}

		// Find face on B most opposing normal (-collisionNormal)
		int tempAxisB;
		bool tempPosB;
		float dotB = -std::numeric_limits<float>::max();
		for (int i = 0; i < 3; ++i)
		{
			float d = glm::dot(obbB.axes[i], -collisionNormal);
			if (d > dotB)
			{
				dotB = d;
				tempAxisB = i;
				tempPosB = true;
			}
			d = glm::dot(-obbB.axes[i], -collisionNormal);
			if (d > dotB)
			{
				dotB = d;
				tempAxisB = i;
				tempPosB = false;
			}
		}

		// Reference object has the face most opposing the normal (A->B direction)
		// Tie break: Arbitrarily choose A. Could use other heuristics (e.g., smaller object incident).
		if (dotA <= dotB)
		{ // A is reference
			refOBB = &obbA;
			incOBB = &obbB;
			refAxisIdx = tempAxisA;
			refPositiveFace = tempPosA;
			incAxisIdx = tempAxisB;
			incPositiveFace = tempPosB;
			// Find incident face on B (most *aligned* with normal)
		}
		else
		{ // B is reference
			refOBB = &obbB;
			incOBB = &obbA;
			refAxisIdx = tempAxisB;
			refPositiveFace = tempPosB;
			incAxisIdx = tempAxisA;
			incPositiveFace = tempPosA;
			// Find incident face on A (most *aligned* with normal)
		}
		// incOBB->findSupportFace(collisionNormal, incAxisIdx, incPositiveFace);
		// refOBB->findSupportFace(-collisionNormal, refAxisIdx, refPositiveFace);


		std::vector<glm::vec3> refVerticesA = refOBB->getFaceVertices(refAxisIdx, refPositiveFace);
		std::vector<glm::vec3> incVerticesB = incOBB->getFaceVertices(incAxisIdx, incPositiveFace);

		// glm::vec3 refFaceNormalA = refOBB->axes[refAxisIdx] * (refPositiveFace ? 1.0f : -1.0f);
		// glm::vec3 refFaceCenterA = refOBB->center + refFaceNormalA * refOBB->halfExtents[refAxisIdx];

		std::vector<glm::vec3> penetratingPointsRaw;
		penetratingPointsRaw.reserve(8); // Max 4 from A, 4 from B

		// Check B's incident face vertices against A's volume
		for (const glm::vec3& vB : incVerticesB)
		{
			if (refOBB->isPointInside(vB))
			{
				penetratingPointsRaw.push_back(vB);
			}
		}

		// Check A's reference face vertices against B's volume
		for (const glm::vec3& vA : refVerticesA)
		{
			if (incOBB->isPointInside(vA))
			{
				penetratingPointsRaw.push_back(vA);
			}
		}


		std::unordered_set<glm::vec3> uniqueProjectedPoints; // Use set to handle duplicates

		if (!penetratingPointsRaw.empty())
		{
			for (const glm::vec3& p : penetratingPointsRaw)
			{
				// // Project the penetrating point onto the reference face plane (A's face)
				// // along the reference face normal.
				// float distToRefPlane = glm::dot(p - refFaceCenterA, refFaceNormalA);

				// // Only consider points that are actually behind or on the reference plane
				// if (distToRefPlane <= 0)
				// {
				// 	glm::vec3 projectedPoint = p - refFaceNormalA * distToRefPlane;
				uniqueProjectedPoints.insert(p);
				// }
			}
		}


		// If vertex checks yield no points, use a fallback
		if (uniqueProjectedPoints.empty())
		{
			// // Fallback: Midpoint between OBB centers pushed back along normal
			// glm::vec3 midPoint = obbA.center + (obbB.center - obbA.center) * 0.5f;
			// // Push back from midpoint towards A along the collision normal
			// outManifold.contactPoints.push_back(midPoint - outManifold.normal * outManifold.penetrationDepth * 0.5f);
			return false;
		}
		else
		{
			// Add unique projected points up to the limit (e.g., 4)
			int count = 0;
			for (const auto& pt : uniqueProjectedPoints)
			{
				if (count < 4)
				{
					outManifold.contactPoints.push_back(pt);
					count++;
				}
				else
				{
					break;
				}
			}
			// Ensure at least one point if any were found
			if (outManifold.contactPoints.empty() && !uniqueProjectedPoints.empty())
			{
				outManifold.contactPoints.push_back(*uniqueProjectedPoints.begin());
			}
		}


		outManifold.contactCount = static_cast<int>(outManifold.contactPoints.size());
	}
	return outManifold.colliding;
}

void projectBoxOntoAxis(const glm::vec3& center, const glm::vec3 axes[3], const glm::vec3& halfExtents, const glm::vec3& axis, float& minProj, float& maxProj)
{
	float centerProj = glm::dot(center, axis);
	float extent = 0.0f;
	for (int i = 0; i < 3; ++i)
	{
		extent += std::abs(glm::dot(axes[i], axis)) * halfExtents[i];
	}
	minProj = centerProj - extent;
	maxProj = centerProj + extent;
}

void computeContactPoints(const OBB& boxA, const OBB& boxB, const glm::vec3& normal, std::vector<glm::vec3>& outPoints)
{
	std::vector<glm::vec3> verticesA;
	std::vector<glm::vec3> verticesB;
	for (int i = -1; i <= 1; i += 2)
	{
		for (int j = -1; j <= 1; j += 2)
		{
			for (int k = -1; k <= 1; k += 2)
			{
				glm::vec3 localA = glm::vec3(i * boxA.halfExtents.x, j * boxA.halfExtents.y, k * boxA.halfExtents.z);
				glm::vec3 worldA = boxA.center + boxA.axes[0] * localA.x + boxA.axes[1] * localA.y + boxA.axes[2] * localA.z;
				verticesA.push_back(worldA);
				glm::vec3 localB = glm::vec3(i * boxB.halfExtents.x, j * boxB.halfExtents.y, k * boxB.halfExtents.z);
				glm::vec3 worldB = boxB.center + boxB.axes[0] * localB.x + boxB.axes[1] * localB.y + boxB.axes[2] * localB.z;
				verticesB.push_back(worldB);
			}
		}
	}
	for (const glm::vec3& v : verticesA)
	{
		glm::vec3 toB = v - boxB.center;
		if (glm::dot(toB, normal) < 0.01f)
		{
			outPoints.push_back(v);
		}
		if (outPoints.size() >= 4)
			break;
	}
	if (outPoints.size() < 4)
	{
		for (const glm::vec3& v : verticesB)
		{
			glm::vec3 toA = v - boxA.center;
			if (glm::dot(toA, -normal) < 0.01f)
			{
				outPoints.push_back(v);
			}
			if (outPoints.size() >= 4)
				break;
		}
	}
}

/**
 * @brief Calculates Time of Impact (TOI) between two moving Boxes using Swept AABB checks
 * @details Focuses on linear relative velocity. Rotational effects are approximated.
 * @param boxColliderA First box collider.
 * @param boxColliderB Second box collider.
 * @param dt Time interval to check for collision.
 * @return TOIResult Containing toi, collision status, and manifold at toi.
 */
TOIResult PhysicsScene::findTOIBoxBox(Collider* boxColliderA, Collider* boxColliderB, float dt)
{
	// Poor TOI results below
	TOIResult result;
	result.manifold = CollisionMannifold(boxColliderA, boxColliderB);
	result.toi = dt;
	result.colliding = false;
	if (!boxColliderA || !boxColliderB || !boxColliderA->getOwnerRigidBody() || !boxColliderB->getOwnerRigidBody())
	{
		return result;
	}
	if (!boxColliderA->getColliderInfo().shapeData || !boxColliderB->getColliderInfo().shapeData)
	{
		return result;
	}
	if (dt <= 0.0f)
	{
		dt = 0.0f;
		result.toi = 0.0f;
	}
	auto rbA = boxColliderA->getOwnerRigidBody();
	auto rbB = boxColliderB->getOwnerRigidBody();
	auto posA0 = rbA->getPosition();
	auto ornA0 = rbA->getOrientation();
	auto linVelA = rbA->getLinearVelocity();
	auto linAccA = rbA->getLinearAcceleration();
	auto angVelA = rbA->getAngularVelocity();
	auto halfExtentsA = boxColliderA->getColliderInfo().shapeData->getHalfExtents();
	auto posB0 = rbB->getPosition();
	auto ornB0 = rbB->getOrientation();
	auto linVelB = rbB->getLinearVelocity();
	auto angVelB = rbB->getAngularVelocity();
	auto halfExtentsB = boxColliderB->getColliderInfo().shapeData->getHalfExtents();
	OBB obbA0(posA0, ornA0, halfExtentsA);
	OBB obbB0(posB0, ornB0, halfExtentsB);
	glm::vec3 initialSeparationVector = obbB0.center - obbA0.center;
	glm::vec3 v_rel = linVelB - linVelA;
	float min_toi = (std::numeric_limits<float>::max)();
	glm::vec3 toi_normal = {0.0f, 0.0f, 0.0f};
	bool potential_collision_found = false;
	bool check_initial_overlap = true;
	// --- Generate Axes to Test (SAT) ---
	glm::vec3 axesToTest[15];
	int axisCount = 0;
	// Face normals of A
	axesToTest[axisCount++] = obbA0.axes[0];
	axesToTest[axisCount++] = obbA0.axes[1];
	axesToTest[axisCount++] = obbA0.axes[2];
	// Face normals of B
	axesToTest[axisCount++] = obbB0.axes[0];
	axesToTest[axisCount++] = obbB0.axes[1];
	axesToTest[axisCount++] = obbB0.axes[2];
	// Edge-Edge cross products
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			glm::vec3 crossProduct = glm::cross(obbA0.axes[i], obbB0.axes[j]);
			// Normalize only if the cross product is not near zero
			if (glm::length2(crossProduct) > 0)
			{
				if (axisCount < 15) // Ensure we don't exceed array bounds
				{
					axesToTest[axisCount++] = glm::normalize(crossProduct);
				}
			}
		}
	}
	// --- Test Each Axis ---
	for (int i = 0; i < axisCount; ++i)
	{
		glm::vec3 L = axesToTest[i]; // Current axis
		// Calculate projected radii at t=0
		float projectedRadiusA = obbA0.projectedRadius(L);
		float projectedRadiusB = obbB0.projectedRadius(L);
		float sumRadii = projectedRadiusA + projectedRadiusB;
		// Calculate signed distance between centers projected onto L at t=0
		float s0 = glm::dot(initialSeparationVector, L);
		// Calculate initial separation distance (gap) along L
		float dist0 = std::abs(s0) - sumRadii;
		if (dist0 < 0)
			continue;
		// If dist0 >= 0, they are separated or touching along this axis projection at t=0.
		// If dist0 < 0, they are overlapping along this axis projection at t=0.
		// Calculate relative translational velocity projected onto L
		float v_trans = glm::dot(v_rel, L);
		// Calculate maximum rate of change of projected radii sum due to angular velocity
		float omega_A_eff = 0.0f;
		for (int ax = 0; ax < 3; ++ax)
		{
			omega_A_eff += halfExtentsA[ax] * std::abs(glm::dot(glm::cross(angVelA, obbA0.axes[ax]), L));
		}
		float omega_B_eff = 0.0f;
		for (int ax = 0; ax < 3; ++ax)
		{
			omega_B_eff += halfExtentsB[ax] * std::abs(glm::dot(glm::cross(angVelB, obbB0.axes[ax]), L));
		}
		float v_rot = omega_A_eff + omega_B_eff; // Max rate sumRadii can increase
		// --- Check for Separation / Calculate Closing Velocity ---
		if (dist0 > 0)
		{
			// Initially separated along this axis projection.
			check_initial_overlap = false; // Found a separating axis at t=0

			// Calculate maximum closing speed along this axis
			float v_closing_max = v_rot - glm::sign(s0) * v_trans;

			if (v_closing_max <= 0)
			{
				// Not approaching (or separating), cannot cause the *first* impact along this axis.
				continue;
			}

			// Estimate TOI for this axis (conservative lower bound)
			float t_axis = dist0 / v_closing_max;

			if (t_axis < min_toi)
			{
				min_toi = t_axis;
				toi_normal = L; // Store axis associated with the minimum TOI
				potential_collision_found = true;
			}
		}
		else
		{
			// Initially touching or overlapping along this axis projection (dist0 <= 0)
			// Check if they are moving apart fast enough to guarantee separation for t > 0
			float separation_speed =
				glm::sign(s0) * v_trans - v_rot; // Min separation speed (or max approach speed if negative)

			if (separation_speed > 0)
			{
				// Moving apart along this axis faster than rotation can counter.
				// This axis guarantees separation for some time t > 0.
				check_initial_overlap = false; // Found a separating axis
				// We don't need to calculate TOI here, as they start separated or touching and move apart.
			}
			// If separation_speed <= 0, they might be moving towards each other or staying touched/overlapped.
			// We don't update min_toi here because the impact time is effectively t=0 if they are moving towards each other.
			// The initial overlap check below will handle this.
		}
	} // End loop over axes

	// --- Final Result Determination ---

	// If a potential TOI < dt was found via a separating axis calculation
	if (potential_collision_found && min_toi < dt && min_toi >= 0.0f)
	{
		// Verify with a static check at the estimated time min_toi
		auto transformA_toi_pair = getTransformsAtTime(boxColliderA->getOwnerRigidBody(), min_toi);
		auto transformB_toi_pair = getTransformsAtTime(boxColliderB->getOwnerRigidBody(), min_toi);
		OBB obbA_toi(transformA_toi_pair.first, transformA_toi_pair.second, halfExtentsA);
		OBB obbB_toi(transformB_toi_pair.first, transformB_toi_pair.second, halfExtentsB);

		CollisionMannifold contactManifold(boxColliderA, boxColliderB);
		if (staticOBBIntersection(obbA_toi, obbB_toi, contactManifold))
		{
			// Static check confirms collision at t = min_toi.
			result.colliding = true;
			result.toi = min_toi;
			result.manifold = contactManifold; // Use manifold from static check

			// Ensure normal points roughly A to B
			glm::vec3 centerDiff_toi = obbB_toi.center - obbA_toi.center;
			if (glm::length2(centerDiff_toi) > 0)
			{
				if (glm::dot(centerDiff_toi, result.manifold.normal) < 0.0f)
				{
					result.manifold.normal = -result.manifold.normal;
				}
			}
			else
			{
				// Centers are very close, use the original separating axis guess
				if (glm::dot(initialSeparationVector, toi_normal) < 0.0f)
				{
					toi_normal = -toi_normal;
				}
				result.manifold.normal = toi_normal; // Fallback to CCD normal
			}

			result.manifold.penetrationDepth = std::max(0.0f, result.manifold.penetrationDepth);
			result.manifold.colliding = true; // Mark manifold as valid
			check_initial_overlap = false; // Collision found, no need for t=0 check anymore
		}
		else
		{
			// Static check at min_toi failed. Conservative TOI was too small.
			// Re-enable the initial overlap check as this axis didn't confirm an impact.
			check_initial_overlap = true;
			// Keep result as non-colliding for now.
		}
	}

	// Perform initial overlap check if needed (no confirmed TOI < dt found, or overlap was possible)
	if (check_initial_overlap)
	{
		CollisionMannifold staticManifold(boxColliderA, boxColliderB);
		if (staticOBBIntersection(obbA0, obbB0, staticManifold))
		{
			// Overlapping or touching at t=0
			result.colliding = true;
			result.toi = 0.0f; // Impact happens immediately
			result.manifold = staticManifold;

			// Ensure normal points roughly A to B
			glm::vec3 centerDiff0 = obbB0.center - obbA0.center;
			if (glm::length2(centerDiff0) > 0)
			{
				if (glm::dot(centerDiff0, result.manifold.normal) < 0.0f)
				{
					result.manifold.normal = -result.manifold.normal;
				}
			} // else: keep normal from static check if centers overlap

			result.manifold.penetrationDepth = std::max(0.0f, result.manifold.penetrationDepth);
			result.manifold.colliding = true; // Mark manifold as valid
		}
		// else: No separating axis found TOI < dt, AND not overlapping at t=0.
		// Result remains non-colliding with toi = dt.
	}

	// If after all checks, result is still not colliding, ensure toi is dt
	if (!result.colliding)
	{
		result.toi = dt;
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
																					 const glm::vec3& triVel2, Collider* meshCollider, float dt)
{
	TOIResult result;
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
		bool p1_inside = (dist1 <= 0);
		bool p2_inside = (dist2 <= 0);
		if (p1_inside && p2_inside)
		{
			clippedVertices.push_back(p2);
		}
		else if (p1_inside && !p2_inside)
		{
			float denom = dist1 - dist2;
			if (std::abs(denom) > 0)
			{
				float t = dist1 / denom;
				if (t >= 0.0f && t <= 1.0f)
					clippedVertices.push_back(p1 + t * (p2 - p1));
			}
		}
		else if (!p1_inside && p2_inside)
		{
			float denom = dist1 - dist2;
			if (std::abs(denom) > 0)
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
	return false;
}
void PhysicsScene::resolveCollisionImpulses(double dt)
{
	for (const CollisionMannifold& manifold : collisionContacts)
	{
		Collider* colliderA = manifold.colliderA;
		Collider* colliderB = manifold.colliderB;
		RigidBody* bodyA = colliderA ? colliderA->getOwnerRigidBody() : nullptr;
		RigidBody* bodyB = colliderB ? colliderB->getOwnerRigidBody() : nullptr;
		if (!bodyA || !bodyB || !colliderA || !colliderB)
			continue;
		if (bodyA->isStatic() && bodyB->isStatic())
			continue;
		if (colliderA->getIsSensor() || colliderB->getIsSensor())
			continue;
		if (manifold.contactPoints.empty())
			continue;
		glm::vec3 averageContactPoint(0.0f);
		averageContactPoint =
			std::accumulate(manifold.contactPoints.begin(), manifold.contactPoints.end(), glm::vec3(0.0f));
		averageContactPoint /= static_cast<float>(manifold.contactPoints.size());
		glm::vec3 rA = averageContactPoint - bodyA->getPosition();
		glm::vec3 rB = averageContactPoint - bodyB->getPosition();
		glm::vec3 velA = bodyA->getLinearVelocity() + glm::cross(bodyA->getAngularVelocity(), rA);
		glm::vec3 velB = bodyB->getLinearVelocity() + glm::cross(bodyB->getAngularVelocity(), rB);
		glm::vec3 relativeVelocity = velB - velA;
		float relativeVelocityNormal = glm::dot(relativeVelocity, manifold.normal);
		if (relativeVelocityNormal > 0.0f)
		{
			continue;
		}
		float restitutionA = colliderA->getPhysicsMaterial().restitution;
		float restitutionB = colliderB->getPhysicsMaterial().restitution;
		float combinedRestitution = (std::max)(restitutionA, restitutionB);
		float invMassA = bodyA->getInverseMass();
		float invMassB = bodyB->getInverseMass();
		glm::mat3 invInertiaTensorA = bodyA->getInverseInertiaTensorWorld();
		glm::mat3 invInertiaTensorB = bodyB->getInverseInertiaTensorWorld();
		glm::vec3 crossRA_N = glm::cross(rA, manifold.normal);
		glm::vec3 crossRB_N = glm::cross(rB, manifold.normal);
		float angInertiaA = (invMassA > 0.0f) ? glm::dot(invInertiaTensorA * crossRA_N, crossRA_N) : 0.0f;
		float angInertiaB = (invMassB > 0.0f) ? glm::dot(invInertiaTensorB * crossRB_N, crossRB_N) : 0.0f;
		float effectiveMassNormal = invMassA + invMassB + angInertiaA + angInertiaB;
		if (effectiveMassNormal <= 1e-6f)
		{
			continue;
		}
		float j = -(1.0f + combinedRestitution) * relativeVelocityNormal / effectiveMassNormal;
		j = std::max(0.0f, j);
		glm::vec3 impulseVec = j * manifold.normal;
		if (bodyA->isDynamic())
		{
			glm::vec3 deltaLinearVelA = -impulseVec * invMassA;
			glm::vec3 deltaAngularVelA = -invInertiaTensorA * glm::cross(rA, impulseVec);
			bodyA->setLinearVelocity(bodyA->getLinearVelocity() + deltaLinearVelA);
			bodyA->setAngularVelocity(bodyA->getAngularVelocity() + deltaAngularVelA);
			bodyA->setSleeping(false);
		}
		if (bodyB->isDynamic())
		{
			glm::vec3 deltaLinearVelB = impulseVec * invMassB;
			glm::vec3 deltaAngularVelB = invInertiaTensorB * glm::cross(rB, impulseVec);
			bodyB->setLinearVelocity(bodyB->getLinearVelocity() + deltaLinearVelB);
			bodyB->setAngularVelocity(bodyB->getAngularVelocity() + deltaAngularVelB);
			bodyB->setSleeping(false);
		}
		// --- TODO: Add friction impulse calculation and application here ---
		// This would typically involve calculating the tangential velocity,
		// calculating the maximum friction impulse based on the normal impulse (j)
		// and the coefficient of friction, and applying a tangential impulse
		// opposing the tangential relative motion, clamped by the maximum friction.
	}
}
void PhysicsScene::synchronizeTransforms() {}
// void PhysicsScene::projectBoxOntoAxis(Collider* boxCollider, const glm::vec3& axis, float& minProj, float& maxProj)
// {
// 	// Ensure valid box and associated data/transform
// 	if (!boxCollider || boxCollider->getShapeType() != ShapeType::Box)
// 	{
// 		minProj = maxProj = 0.0f;
// 		return;
// 	}
// 	const auto* boxData = static_cast<const BoxShapeData*>(boxCollider->getColliderInfo().shapeData.get());
// 	const glm::mat4* worldTransformPtr = boxCollider->getTransform(); // Get from collider's owner RB
// 	if (!boxData || !worldTransformPtr)
// 	{
// 		minProj = maxProj = 0.0f;
// 		return;
// 	}

// 	// Combine world transform with collider's local offset and rotation
// 	// glm::mat4 localOffsetTransform =
// 	// 	glm::translate(glm::mat4(1.0f), boxCollider->getOffset()) * glm::mat4_cast(boxCollider->getOrientationOffset());
// 	glm::mat4 finalTransform = (*worldTransformPtr); // * localOffsetTransform;

// 	// Box half extents
// 	const glm::vec3 h = boxData->halfExtents;

// 	// Calculate the 8 vertices of the box in its local space (relative to collider center)
// 	glm::vec3 localVertices[8] = {{-h.x, -h.y, -h.z}, {h.x, -h.y, -h.z}, {h.x, h.y, -h.z}, {-h.x, h.y, -h.z},
// 																{-h.x, -h.y, h.z},	{h.x, -h.y, h.z},	 {h.x, h.y, h.z},	 {-h.x, h.y, h.z}};

// 	// Initialize min/max projection values
// 	minProj = std::numeric_limits<float>::max();
// 	maxProj = std::numeric_limits<float>::lowest(); // Use lowest() for negative infinity equivalent

// 	// Project each vertex onto the axis
// 	for (int i = 0; i < 8; ++i)
// 	{
// 		// Transform vertex to world space using the combined transform
// 		glm::vec4 worldVertex4 = finalTransform * glm::vec4(localVertices[i], 1.0f);
// 		glm::vec3 worldVertex = glm::vec3(worldVertex4); // Convert from vec4

// 		// Project onto the axis (dot product)
// 		float projection = glm::dot(worldVertex, axis);

// 		// Update min and max
// 		minProj = std::min(minProj, projection);
// 		maxProj = std::max(maxProj, projection);
// 	}
// }
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
	// glm::quat localRot = boxCollider->getOrientationOffset();
	glm::quat finalRot = worldRot; // * localRot; // Combine rotations

	// Convert final rotation quaternion to a 3x3 matrix (or use columns of 4x4)
	glm::mat3 rotationMatrix = glm::mat3_cast(finalRot);

	// Extract axes (columns of the rotation matrix) and normalize
	// Normalization is crucial if the original transform has non-uniform scaling
	worldAxes.push_back(glm::normalize(rotationMatrix[0])); // X-axis
	worldAxes.push_back(glm::normalize(rotationMatrix[1])); // Y-axis
	worldAxes.push_back(glm::normalize(rotationMatrix[2])); // Z-axis

	return worldAxes;
}
