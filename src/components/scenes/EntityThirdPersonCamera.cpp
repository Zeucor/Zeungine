#include <zg/components/scenes/EntityThirdPersonCamera.hpp>
#include <zg/physics/AABB.hpp>
#include <zg/Scene.hpp>
#include <zg/Window.hpp>
#include <zg/Registry.hpp>
using zg::Registry;
zg::components::scenes::SceneComponentCreateInfo zg::components::scenes::EntityThirdPersonCameraFactory(Entity& entity)
{
    zg::components::scenes::SceneComponentCreateInfo info{
        .name = "EntityThirdPersonCamera",
        .onAttachedFunction = [entityIndexStack = entity.INDEX_STACK](auto& component)
        {
            auto& scene = Registry::GetSingleton().getScene(component.HOST_INDEX_STACK);
            auto& focused = component.template make<bool>("Focused", false);
            auto& mouseMoveID = component.template make<UniqueIdentifier>("MouseMoveID", 0);
            auto& focusID = component.template make<UniqueIdentifier>("FocusID", 0);
            auto& distance = component.template make<float>("Distance", 8.f);
            auto& verticalOffset = component.template make<float>("VerticalOffset", 1.5f);
            auto& mouseSensitivity = component.template make<float>("MouseSensitivity", 0.1f);
            auto& currentYaw = component.template make<float>("CurrentYaw", 0.0f);
            auto& currentPitch = component.template make<float>("CurrentPitch", -22.5f);
            auto& minPitch = component.template make<float>("MinPitch", -90.0f);
            auto& maxPitch = component.template make<float>("MaxPitch", -5.0f);
            auto& lastPosition = component.template make<glm::vec2>("LastPosition", 0.0f, 0.0f);
            auto& deadZonePercent = component.template make<float>("DeadZonePercent", 0.1f);
            auto& window = Registry::GetSingleton().getWindow(component.HOST_INDEX_STACK);
            mouseMoveID = window.addMouseMoveHandler([HOST_INDEX_STACK = component.HOST_INDEX_STACK, componentID = component.ID](glm::vec2 coords)
            	{
                    auto& scene = Registry::GetSingleton().getScene(HOST_INDEX_STACK);
                    auto& window = Registry::GetSingleton().getWindow(HOST_INDEX_STACK);
                    auto& component = scene.getComponentByID(componentID);
            		if (!window.focused)
            		{
            			return;
            		}
                    auto& distance = component.template getData<float>("Distance");
                    auto& verticalOffset = component.template getData<float>("VerticalOffset");
                    auto& mouseSensitivity = component.template getData<float>("MouseSensitivity");
                    auto& currentYaw = component.template getData<float>("CurrentYaw");
                    auto& currentPitch = component.template getData<float>("CurrentPitch");
                    auto& minPitch = component.template getData<float>("MinPitch");
                    auto& maxPitch = component.template getData<float>("MaxPitch");
                    auto& lastPosition = component.template getData<glm::vec2>("LastPosition");
                    auto& deadZonePercent = component.template getData<float>("DeadZonePercent");
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
            		glm::vec2 center = {window.windowWidth / 2.0f, window.windowHeight / 2.0f};
            		float boxHalfWidth = window.windowWidth * (deadZonePercent * 0.5f);
            		float boxHalfHeight = window.windowHeight * (deadZonePercent * 0.5f);
            		zg::physics::AABB<2> centerBox(glm::vec2(center.x - boxHalfWidth, center.y - boxHalfHeight),
            																	 glm::vec2(center.x + boxHalfWidth, center.y + boxHalfHeight));
        
            		if (!centerBox.isPointInside(coords))
            		{
            			window.warpPointer(center);
            			lastPosition = center;
            		}
            		else
            		{
            			lastPosition = coords;
            		}
            	});
            focusID =
                window.addFocusHandler([entityIndexStack, HOST_INDEX_STACK = component.HOST_INDEX_STACK, componentID = component.ID](bool focused)
                	{
                        auto& scene = Registry::GetSingleton().getScene(HOST_INDEX_STACK);
                        auto& window = Registry::GetSingleton().getWindow(HOST_INDEX_STACK);
                        auto& component = scene.getComponentByID(componentID);
                        auto& entity = Registry::GetSingleton().getEntity(entityIndexStack);
                		if (focused)
                		{
                            auto& currentYaw = component.template getData<float>("CurrentYaw");
                            auto& currentPitch = component.template getData<float>("CurrentPitch");
                            auto& lastPosition = component.template getData<glm::vec2>("LastPosition");
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
            
                			if (window.iPlatformWindow)
                				window.iPlatformWindow->hidePointer();
            
                			// Initialize lastPosition and warp pointer
                			glm::vec2 center = {window.windowWidth / 2.0f, window.windowHeight / 2.0f};
                			lastPosition = center;
                			window.warpPointer(center);
                		}
                		else
                		{
                			if (window.iPlatformWindow)
                				window.iPlatformWindow->showPointer();
                		}
                	}
                
                );
        },
        .onDetachedFunction = [](auto& component)
        {
            auto& mouseMoveID = component.template getData<UniqueIdentifier>("MouseMoveID");
            auto& focusID = component.template getData<UniqueIdentifier>("FocusID");
            auto& scene = Registry::GetSingleton().getScene(component.HOST_INDEX_STACK);
            auto& window = Registry::GetSingleton().getWindow(component.HOST_INDEX_STACK);
            window.removeMouseMoveHandler(mouseMoveID);
            window.removeFocusHandler(focusID);
        },
        .onUpdateFunction = [entityIndexStack = entity.INDEX_STACK](auto& component)
        {
            auto& distance = component.template getData<float>("Distance");
            auto& verticalOffset = component.template getData<float>("VerticalOffset");
            auto& currentYaw = component.template getData<float>("CurrentYaw");
            auto& currentPitch = component.template getData<float>("CurrentPitch");
            auto& scene = Registry::GetSingleton().getScene(component.HOST_INDEX_STACK);
            auto& entity = Registry::GetSingleton().getEntity(entityIndexStack);
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
            view.setDirty();

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
    };
    return info;
}