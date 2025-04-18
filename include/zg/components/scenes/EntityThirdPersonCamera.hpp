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
		bool focused = false;
		UniqueIdentifier mouseMoveID = 0;
		UniqueIdentifier focusID = 0;
		Scene& scene;
		Entity& entity;
		float distance = 8.f;
		float verticalOffset = 1.5f;
		float mouseSensitivity = 0.1f;
		float currentYaw = 0.0f; // Camera Yaw
		float currentPitch = -22.5f;
		float minPitch = -90.0f;
		float maxPitch = -5.0f;
		glm::vec2 lastPosition = {0.0f, 0.0f};
		float deadZonePercent = 0.1f;

		EntityThirdPersonCamera(Scene& scene, Entity& entity) :
				ISceneComponent("EntityThirdPersonCamera"), scene(scene), entity(entity)
		{
			mouseMoveID = scene.window.addMouseMoveHandler(
				std::bind(&EntityThirdPersonCamera::mouseMoveHandler, this, std::placeholders::_1));
			focusID =
				scene.window.addFocusHandler(std::bind(&EntityThirdPersonCamera::focusHandler, this, std::placeholders::_1));
			focusHandler(true);
		}

		~EntityThirdPersonCamera()
		{
			if (mouseMoveID != 0)
			{
				scene.window.removeMouseMoveHandler(mouseMoveID);
			}
			if (focusID != 0)
			{
				scene.window.removeFocusHandler(focusID);
			}
		}

		void onAttached() override {}

		void onUpdate() override
		{
			if (!scene.viewPointer)
				return;

			zg::vp::View& view = *scene.viewPointer;
			glm::vec3& viewPosition = view.position;
			glm::vec3& viewDirection = view.direction;
			glm::vec3 targetBasePosition = entity.position;

			glm::vec3 lookAtTarget = targetBasePosition + glm::vec3(0.0f, verticalOffset, 0.0f);

			// Calculate camera position based on currentYaw/currentPitch
			float cameraYawRad = glm::radians(currentYaw);
			float cameraPitchRad = glm::radians(currentPitch);
			glm::vec3 offset = glm::vec3(0.0f, 0.0f, distance);
			glm::quat pitchQuat = glm::angleAxis(cameraPitchRad, glm::vec3(1.0f, 0.0f, 0.0f));
			glm::quat yawQuat = glm::angleAxis(cameraYawRad, glm::vec3(0.0f, 1.0f, 0.0f));
			offset = yawQuat * pitchQuat * offset;
			viewPosition = lookAtTarget + offset;
			viewDirection = glm::normalize(lookAtTarget - viewPosition);
			view.update();

			// --- Update Entity Yaw Rotation ---
			glm::vec3 horizontalOffset =
				glm::vec3(viewPosition.x - targetBasePosition.x, 0.0f, viewPosition.z - targetBasePosition.z);

			if (glm::length2(horizontalOffset) > 1e-5f)
			{
				// Calculate target yaw based on camera position (range [-pi, pi])
				// This is the direction the entity should face (-horizontalOffset)
				float targetEntityYawRad = atan2(-horizontalOffset.x, -horizontalOffset.z);

				glm::quat currentEntityRot = entity.rotation;

				// --- Get current entity yaw using forward vector ---
				glm::mat4 currentRotMat = glm::mat4_cast(currentEntityRot);
				// IMPORTANT: Assumes model's forward is +Z. If it's +X use [0], -Z use -[2], etc.
				glm::vec3 currentForward = glm::normalize(glm::vec3(currentRotMat[2]));
				// Calculate yaw angle from the forward vector's projection on XZ plane (relative to +Z)
				float currentEntityYawRad = atan2(currentForward.x, currentForward.z);
				// ----------------------------------------------------

				// Calculate the raw difference in yaw
				float deltaYawRad = targetEntityYawRad - currentEntityYawRad;

				// Normalize the angle difference to the shortest path range [-pi, pi]
				while (deltaYawRad > glm::pi<float>())
				{
					deltaYawRad -= 2.0f * glm::pi<float>();
				}
				while (deltaYawRad <= -glm::pi<float>())
				{
					deltaYawRad += 2.0f * glm::pi<float>();
				}

				// Create a rotation quaternion for the shortest angle difference
				glm::quat yawRotationDelta = glm::angleAxis(deltaYawRad, glm::vec3(0.0f, 1.0f, 0.0f));
				// Apply the delta rotation (pre-multiplication preserves existing pitch/roll) and normalize
				entity.setOrientation(glm::normalize(yawRotationDelta * currentEntityRot));
			}
		}

		void mouseMoveHandler(glm::vec2 coords)
		{
			if (!scene.window.focused)
			{
				return;
			}

			glm::vec2 delta = coords - lastPosition;

			// Update Camera Yaw/Pitch
			currentYaw -= delta.x * mouseSensitivity;
			currentYaw = fmod(currentYaw, 360.0f);
			if (currentYaw < 0.0f)
			{
				currentYaw += 360.0f;
			}

			currentPitch += delta.y * mouseSensitivity;
			currentPitch = std::clamp(currentPitch, minPitch, maxPitch);

			// Conditional Warp Logic
			glm::vec2 center = {scene.window.windowWidth / 2.0f, scene.window.windowHeight / 2.0f};
			float boxHalfWidth = scene.window.windowWidth * (deadZonePercent * 0.5f);
			float boxHalfHeight = scene.window.windowHeight * (deadZonePercent * 0.5f);
			zg::physics::AABB<2> centerBox(glm::vec2(center.x - boxHalfWidth, center.y - boxHalfHeight),
																		 glm::vec2(center.x + boxHalfWidth, center.y + boxHalfHeight));

			if (!centerBox.isPointInside(coords))
			{
				scene.window.warpPointer(center);
				lastPosition = center;
			}
			else
			{
				lastPosition = coords;
			}
		}


		void focusHandler(bool focused)
		{
			if (focused)
			{
				// Sync camera yaw FROM entity when gaining focus
				glm::quat currentEntityRot = entity.rotation;

				// --- Get current entity yaw using forward vector on focus ---
				glm::mat4 currentRotMat = glm::mat4_cast(currentEntityRot);
				// IMPORTANT: Assumes model's forward is +Z. Adjust index [2] if needed.
				glm::vec3 currentForward = glm::normalize(glm::vec3(currentRotMat[2]));
				float currentEntityYawRad = atan2(currentForward.x, currentForward.z);
				// -----------------------------------------------------------

				// Convert entity yaw to degrees and set camera yaw
				// Relationship: CameraYaw = EntityYaw + 180 degrees (approx)
				currentYaw = glm::degrees(currentEntityYawRad) + 180.0f;

				// Normalize camera yaw to [0, 360)
				currentYaw = fmod(currentYaw, 360.0f);
				if (currentYaw < 0.0f)
				{
					currentYaw += 360.0f;
				}

				if (scene.window.iPlatformWindow)
					scene.window.iPlatformWindow->hidePointer();

				// Initialize lastPosition and warp pointer
				glm::vec2 center = {scene.window.windowWidth / 2.0f, scene.window.windowHeight / 2.0f};
				lastPosition = center;
				scene.window.warpPointer(center);
			}
			else
			{
				if (scene.window.iPlatformWindow)
					scene.window.iPlatformWindow->showPointer();
			}
		}

		void onDetached() override {}
	};
} // namespace zg::components::scenes
