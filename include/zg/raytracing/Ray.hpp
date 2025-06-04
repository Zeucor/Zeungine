#pragma once
#include <zg/glm.hpp>
namespace zg::raytracing
{
    inline glm::vec3 unProject(glm::vec3 win, const glm::mat4& inverseProjectionView, glm::vec4 viewport)
    {
        glm::vec4 tmp = glm::vec4(win, 1.0f);
        // Convert screen coordinates to NDC
        tmp.x = (tmp.x - viewport[0]) / viewport[2] * 2.0f - 1.0f;
        tmp.y = (tmp.y - viewport[1]) / viewport[3] * 2.0f - 1.0f;
        // Map win.z from [0, 1] to NDC [0, 1] for Vulkan/DirectX
        tmp.z = win.z; // Or glm::clamp(win.z, 0.0f, 1.0f); for safety

        glm::vec4 obj = inverseProjectionView * tmp; // Transform into world space
        obj /= obj.w; // Perform perspective divide
        return glm::vec3(obj);
    }
    inline glm::vec3 unProjectToView(glm::vec3 win, const glm::mat4& inverseProjection, glm::vec4 viewport)
    {
        glm::vec4 tmp = glm::vec4(win, 1.0f);
        // Convert screen coordinates to NDC
        tmp.x = (tmp.x - viewport[0]) / viewport[2] * 2.0f - 1.0f;
        tmp.y = (tmp.y - viewport[1]) / viewport[3] * 2.0f - 1.0f;
        // Map win.z from [0, 1] to NDC [0, 1] for Vulkan/DirectX
        // This mapping is correct for your glm::orthoRH_ZO projection
        tmp.z = win.z;

        glm::vec4 viewPos = inverseProjection * tmp; // Transform into view space
        viewPos /= viewPos.w; // Perform perspective divide (though for ortho, w should be 1)
        return glm::vec3(viewPos);
    }
    template <typename RayT>
    inline RayT mouseCoordToRay(uint32_t windowHeight, glm::vec2 screenCoord, glm::vec4 viewport, const glm::mat4& projection,
                                const glm::mat4& view, float nearPlane, float farPlane)
    {
        screenCoord.y = (windowHeight - screenCoord.y) - 1;
        // Calculate inverse matrices separately
        glm::mat4 inverseProjection = glm::inverse(projection);
        glm::mat4 inverseView = glm::inverse(view);

        // Unproject points to view space
        glm::vec3 nearPointView = unProjectToView(glm::vec3(screenCoord, 0.0), inverseProjection, viewport);
        glm::vec3 farPointView = unProjectToView(glm::vec3(screenCoord, 0.99999), inverseProjection, viewport);

        // Transform view space points to world space
        glm::vec3 nearPointWorld = glm::vec3(inverseView * glm::vec4(nearPointView, 1.0));
        glm::vec3 farPointWorld = glm::vec3(inverseView * glm::vec4(farPointView, 1.0));

        // Calculate ray direction and create the ray
        glm::vec3 rayDir = glm::normalize(farPointWorld - nearPointWorld);
        RayT ray;
        ray.org[0] = nearPointWorld.x;
        ray.org[1] = nearPointWorld.y;
        ray.org[2] = nearPointWorld.z;
        ray.dir[0] = rayDir.x;
        ray.dir[1] = rayDir.y;
        ray.dir[2] = rayDir.z;
        ray.tmin = nearPlane;
        ray.tmax = farPlane;
        return ray;
    }
    template <typename RayT>
    inline RayT mouseCoordToRayInverse(uint32_t windowHeight, glm::vec2 screenCoord, glm::vec4 viewport, const glm::mat4& inverseProjection,
                                const glm::mat4& inverseView, float nearPlane, float farPlane)
    {
        screenCoord.y = (windowHeight - screenCoord.y) - 1;

        // Unproject points to view space
        glm::vec3 nearPointView = unProjectToView(glm::vec3(screenCoord, 0.0), inverseProjection, viewport);
        glm::vec3 farPointView = unProjectToView(glm::vec3(screenCoord, 0.99999), inverseProjection, viewport);

        // Transform view space points to world space
        glm::vec3 nearPointWorld = glm::vec3(inverseView * glm::vec4(nearPointView, 1.0));
        glm::vec3 farPointWorld = glm::vec3(inverseView * glm::vec4(farPointView, 1.0));

        // Calculate ray direction and create the ray
        glm::vec3 rayDir = glm::normalize(farPointWorld - nearPointWorld);
        RayT ray;
        ray.org[0] = nearPointWorld.x;
        ray.org[1] = nearPointWorld.y;
        ray.org[2] = nearPointWorld.z;
        ray.dir[0] = rayDir.x;
        ray.dir[1] = rayDir.y;
        ray.dir[2] = rayDir.z;
        ray.tmin = nearPlane;
        ray.tmax = farPlane;
        return ray;
    }
}
namespace zg::exp::raytracing
{
    struct Ray
    {
        float org[3] = {0};
        float dir[3] = {0};
        float tmin = 0;
        float tmax = 0;
    };
}