#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <algorithm>
#include <zg/glm.hpp>
#include <zg/physics/AABB.hpp>
using zg::physics::AABB;

// --- Collision Result Structure ---
struct CollisionResult {
    bool collided = false;
    float toi = std::numeric_limits<float>::infinity(); // Time of Impact
    // Optional: Add normal of collision if needed
};

// --- Swept AABB Collision Function ---
// Calculates the Time of Impact (TOI) for two linearly moving AABBs.
// Assumes velocities are constant over the deltaTime.
// deltaTime is typically 1.0 for normalized calculations, result toi is then scaled.
CollisionResult SweptAABB(
    const AABB& a, const glm::vec3& vel_a,
    const AABB& b, const glm::vec3& vel_b,
    float deltaTime = 1.0)
{
    CollisionResult result;
    result.toi = deltaTime; // Assume no collision within interval initially

    // Calculate relative velocity (treat 'b' as stationary, 'a' moves with 'v')
    glm::vec3 v = vel_a - vel_b;

    // --- Calculate time intervals for overlap along each axis ---

    float t_enter = 0.0; // Latest time collision interval starts
    float t_exit = deltaTime; // Earliest time collision interval ends

    // Small epsilon to handle floating point comparisons and division by zero
    const float epsilon = 0;

    // Check each axis (X, Y, Z)
    for (int i = 0; i < 3; ++i) {
        float v_i = 0.0;
        float a_min_i = 0.0, a_max_i = 0.0;
        float b_min_i = 0.0, b_max_i = 0.0;

        // Extract axis-specific values
        if (i == 0) { // X-axis
            v_i = v.x; a_min_i = a._min.x; a_max_i = a._max.x; b_min_i = b._min.x; b_max_i = b._max.x;
        } else if (i == 1) { // Y-axis
            v_i = v.y; a_min_i = a._min.y; a_max_i = a._max.y; b_min_i = b._min.y; b_max_i = b._max.y;
        } else { // Z-axis
            v_i = v.z; a_min_i = a._min.z; a_max_i = a._max.z; b_min_i = b._min.z; b_max_i = b._max.z;
        }

        // Check for initial separation/overlap and zero relative velocity
        if (std::abs(v_i) < epsilon) {
            // Relative velocity is zero on this axis.
            // If they are separated on this axis, they will never collide.
            if (a_max_i < b_min_i || b_max_i < a_min_i) {
                result.collided = false;
                result.toi = deltaTime; // No collision possible within interval
                return result; // Early exit
            }
            // If they overlap and velocity is zero, this axis doesn't restrict the collision time further.
            // The interval for this axis is effectively [0, deltaTime].
        } else {
            // Calculate times when the boundaries align on this axis
            // Time for a._max to reach b._min
            float t1 = (b_min_i - a_max_i) / v_i;
            // Time for a._min to reach b._max
            float t2 = (b_max_i - a_min_i) / v_i;

            // Sort the times to find the interval of overlap [t_near, t_far] for this axis
            float t_near = std::min(t1, t2);
            float t_far = std::max(t1, t2);

            // Update the overall collision interval [t_enter, t_exit]
            // We need the intersection of the intervals from all axes.
            t_enter = std::max(t_enter, t_near); // Latest start time
            t_exit = std::min(t_exit, t_far);   // Earliest end time

            // Check for immediate non-collision based on this axis
            if (t_enter > t_exit) {
                 // The combined interval is invalid, meaning no overlap is possible
                result.collided = false;
                result.toi = deltaTime;
                return result; // Early exit
            }
        }
    }

    // --- Final Check ---
    // After checking all axes, if t_enter <= t_exit, a potential collision exists
    // within the time interval [t_enter, t_exit].

    // We also need t_enter to be within our desired time frame [0, deltaTime]
    // and t_exit must be positive (otherwise interval is entirely in the past)
    if (t_enter >= 0.0 && t_enter < deltaTime && t_enter <= t_exit) {
        result.collided = true;
        result.toi = t_enter;
    } else {
        // Collision happens outside the desired [0, deltaTime] interval
        // or the interval calculation resulted in no valid overlap time.
        result.collided = false;
        result.toi = deltaTime; // Indicate no collision *within* the interval
    }

    // Note: If t_enter is exactly 0, it means they were touching or overlapping initially.
    // A check for initial overlap could be done separately at the beginning for clarity,
    // but this logic handles it correctly as well.

    return result;
}


// --- Example Usage ---
int main() {
    // Define two AABBs
    AABB box_a = {{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}}; // Unit cube at origin
    AABB box_b = {{5.0, 0.0, 0.0}, {6.0, 1.0, 1.0}}; // Unit cube at x=5

    // Define velocities
    glm::vec3 vel_a = {2.0, 0.0, 0.0}; // Box A moves towards Box B
    glm::vec3 vel_b = {0.0, 0.0, 0.0}; // Box B is stationary

    // Set time interval (e.g., one frame or one second)
    float dt = 3.0; // Check for collision over the next 3 time units

    // Perform the swept AABB test
    CollisionResult collision = SweptAABB(box_a, vel_a, box_b, vel_b, dt);

    // Output the result
    if (collision.collided) {
        std::cout << "Collision detected!" << std::endl;
        std::cout << "Time of Impact (TOI): " << collision.toi << std::endl;

        // Calculate collision point (optional, approximate)
        // Position at TOI = initial_pos + velocity * toi
        glm::vec3 collision_pos_a = box_a._min + (box_a._max - box_a._min) * 0.5f + vel_a * collision.toi; // Center of A at TOI
        glm::vec3 collision_pos_b = box_b._min + (box_b._max - box_b._min) * 0.5f + vel_b * collision.toi; // Center of B at TOI
        std::cout << "Approx. center of A at TOI: (" << collision_pos_a.x << ", " << collision_pos_a.y << ", " << collision_pos_a.z << ")" << std::endl;
        std::cout << "Approx. center of B at TOI: (" << collision_pos_b.x << ", " << collision_pos_b.y << ", " << collision_pos_b.z << ")" << std::endl;

    } else {
        std::cout << "No collision within the time interval (" << dt << ")." << std::endl;
    }

    return 0;
}