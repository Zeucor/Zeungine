#pragma once
#include <zg/Entity.hpp>
#include <zg/Scene.hpp>
#include <zg/glm.hpp>
#include <zg/interfaces/ISceneComponent.hpp>
#include <zg/vp/View.hpp>
namespace zg
{
	struct Entity;
}
namespace zg::components::scenes
{
	struct EntityThirdPersonCamera : interfaces::ISceneComponent
	{
		UniqueIdentifier mouseMoveID = 0;
		UniqueIdentifier focusID = 0;
		Scene& scene;
		Entity& entity;
		float distance = 8.f;
		float verticalOffset = 1.5f; // Offset the look-at point slightly above the entity's base position
		float mouseSensitivity = 0.1f; // Adjust sensitivity of mouse movement
		// Camera orientation angles (in degrees)
		float currentYaw = 180.0f; // Rotation around the vertical axis (Y-axis)
		float currentPitch = -22.5f; // Rotation around the horizontal axis (X-axis)
		float minPitch = -90.0f; // Minimum vertical angle
		float maxPitch = -5.0f; // Maximum vertical angle

		EntityThirdPersonCamera(Scene& scene, Entity& entity) :
				ISceneComponent("EntityThirdPersonCamera"), scene(scene), entity(entity)
		{
			mouseMoveID = scene.window.addMouseMoveHandler(
				std::bind(&EntityThirdPersonCamera::mouseMoveHandler, this, std::placeholders::_1));
			focusID =
				scene.window.addFocusHandler(std::bind(&EntityThirdPersonCamera::focusHandler, this, std::placeholders::_1));
		}

		~EntityThirdPersonCamera()
		{
			scene.window.removeMouseMoveHandler(mouseMoveID);
			scene.window.removeFocusHandler(focusID);
		}

		void onAttached() override {}

		void onUpdate() override
		{
			// Ensure view pointer is valid
			if (!scene.viewPointer)
				return;

			// Get the current view properties
			zg::vp::View& view = *scene.viewPointer;
			glm::vec3& viewPosition = view.position; // Camera's current position (will be updated)
			glm::vec3& viewDirection = view.direction; // Camera's look direction (will be updated)

			// Get the target entity's base position
			glm::vec3 targetBasePosition = entity.position;

			// Calculate the point the camera should look at (entity position + vertical offset)
			glm::vec3 lookAtTarget = targetBasePosition + glm::vec3(0.0f, verticalOffset, 0.0f);

			// --- Calculate Camera Position based on Orbit ---
			// Convert angles from degrees to radians for trigonometric functions
			float yawRad = glm::radians(currentYaw);
			float pitchRad = glm::radians(currentPitch);

			// Calculate camera position relative to the lookAtTarget using spherical coordinates
			// Start with an offset along the positive Z-axis (behind the target in typical right-handed coords)
			glm::vec3 offset = glm::vec3(0.0f, 0.0f, distance);

			// Apply pitch rotation (around X-axis) - Rotate around the target's local X
			glm::quat pitchQuat = glm::angleAxis(pitchRad, glm::vec3(1.0f, 0.0f, 0.0f));
			// Apply yaw rotation (around Y-axis) - Rotate around the world's Y
			glm::quat yawQuat = glm::angleAxis(yawRad, glm::vec3(0.0f, 1.0f, 0.0f));

			// Combine rotations: Apply yaw first, then pitch relative to the yawed orientation
			// Note: Quaternion multiplication order matters. p * q applies q then p.
			// We want to rotate around world Y (yaw) then local X (pitch).
			// So, calculate the final offset by rotating the initial offset vector.
			offset = yawQuat * pitchQuat * offset; // Apply pitch then yaw to the offset vector

			// Final camera position is the lookAtTarget plus the calculated offset
			viewPosition = lookAtTarget + offset;

			// --- Update View Direction ---
			// The camera should always look at the lookAtTarget point
			viewDirection = glm::normalize(lookAtTarget - viewPosition);

			// --- Update Entity Rotation ---
			// Rotate the entity around its vertical axis (Y) to match the camera's yaw.
			// Create a quaternion representing rotation around the world Y-axis by yawRad radians.
			// We negate yawRad because typically positive yaw means turning right (clockwise around Y),
			// but entity rotation might follow a different convention. Adjust if needed.
			// Or, if the entity should face the *opposite* direction of the camera's forward vector projected onto the XZ
			// plane. Let's assume the entity should face the same direction the camera is looking *horizontally*.
			entity.rotation = glm::angleAxis(-glm::radians(360.f-currentYaw), glm::vec3(0.0f, 1.0f, 0.0f));


			// Update the view's internal state (if necessary)
			view.update(); // Example call, ensure your view updates its matrices
		}

		void mouseMoveHandler(glm::vec2 coords)
		{
			// Ignore mouse movement if the window just warped the pointer
			if (scene.window.justWarpedPointer)
			{
				scene.window.justWarpedPointer = false;
				return;
			}

			// Ignore mouse movement if the window doesn't have focus
			if (!scene.window.focused)
				return;

			// Calculate the center of the window
			glm::vec2 center = {scene.window.windowWidth / 2.0f, scene.window.windowHeight / 2.0f};

			// Calculate the difference (delta) between the current mouse coordinates and the center
			glm::vec2 delta = coords - center;

			// --- Update Yaw and Pitch based on mouse delta ---
			// Adjust yaw based on horizontal mouse movement (delta.x)
			currentYaw -= delta.x * mouseSensitivity;
			// Keep yaw within 0-360 range (optional, but good practice)
			currentYaw = fmod(currentYaw, 360.0f);
			if (currentYaw < 0.0f)
				currentYaw += 360.0f;


			// Adjust pitch based on vertical mouse movement (delta.y)
			// Invert delta.y because typically moving mouse up decreases pitch
			currentPitch += delta.y * mouseSensitivity;

			// Clamp the pitch to prevent the camera from flipping over
			currentPitch = std::clamp(currentPitch, minPitch, maxPitch);

			// --- Don't directly modify viewPointer's phi/theta here ---
			// The onUpdate function now calculates position/direction based on currentYaw/currentPitch

			// Warp the mouse pointer back to the center of the window
			scene.window.warpPointer(center);
		}

		void focusHandler(bool focused)
		{
			if (focused)
				scene.window.iPlatformWindow->hidePointer();
			else
				scene.window.iPlatformWindow->showPointer();
		}

		void onDetached() override {}
	};
} // namespace zg::components::scenes
