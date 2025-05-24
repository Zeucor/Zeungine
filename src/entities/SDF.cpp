#include <zg/entities/SDF.hpp>
#include <zg/Mesh.hpp>
#include <zg/utilities.hpp>
using namespace zg;
int32_t SDFRegistry::register_sdf(const std::string& key, const shaders::Shader::ShaderHook& functionHook, const std::string& param_append_string)
{
    auto iter = functionHooks.find(key);
    if (iter != functionHooks.end())
    {
        throw std::runtime_error("SDF with key: '" + key + "' already registered!");
    }
    auto id = total_sdf_count++;
    functionHooks[key] = {id, functionHook, param_append_string};
    return id;
}
int32_t SDFRegistry::get_sdf_type(const std::string& key)
{
    auto iter = functionHooks.find(key);
    if (iter == functionHooks.end())
    {
        return -1;
    }
    auto& pair = iter->second;
    return std::get<0>(pair);
}
void SDFRegistry::register_c_sdf(int32_t id, const sdf_function& sdf)
{
    cFunctions.emplace(id, sdf);
}
sdf_function& SDFRegistry::get_sdf_function(int32_t id)
{
    auto iter = cFunctions.find(id);
    if (iter == cFunctions.end())
        throw std::runtime_error("c SDF is not registered");
    return iter->second;
}
size_t SDFRegistry::size()
{
    return functionHooks.size();
}
bool registered_zg_sdfs = false;
zg::EntityCreateInfo zg::entities::SDFFactory(const std::string& sdf_key, glm::vec4 color, const std::string& name, glm::vec3 position, glm::quat rotation, glm::vec3 scale,
    const shaders::RuntimeConstants constants)
{
    if (!registered_zg_sdfs)
        throw std::runtime_error("registered_zg_sdfs is false");
    auto sdf_type = SDFRegistry::GetSingleton().get_sdf_type(sdf_key);
    auto mergedConstants = shaders::mergeConstants({
        shaders::RuntimeConstants({"Shape", "Color"}),
        shaders::common_zg_constants,
        constants
    });
    zg::MeshCreateInfo meshInfo{
        .name = "SDF",
        .shapeType = ShapeType::SDF,
        .material = {
            color,
            0
        },
        .info = [](auto&) -> MeshInfo {
            return { };
        },
        .constants = mergedConstants,
        .meta_int = sdf_type
    };
    zg::EntityCreateInfo info{
        .typeName = sdf_key + " SDF",
        .position = position,
        .rotation = rotation,
        .scale = scale,
        .name = name,
        .meshInfos = {
            meshInfo
        }
    };
    return info;
}
void zg::register_zg_sdfs()
{
    auto& sdf_rgy = SDFRegistry::GetSingleton();
    auto sphere_id = sdf_rgy.register_sdf("Sphere", [](auto& shader, auto& constants) {
        return R"(float SphereSDF(vec3 p_local) {
    return length(p_local) - (0.5);
})";
    });
    sdf_rgy.register_c_sdf(sphere_id, [](auto& entity, auto p) {
        return glm::length(p) - 0.5f;
    });
    auto cube_id = sdf_rgy.register_sdf("Cube", [](auto& shader, auto& constants) {
        return R"(float CubeSDF(vec3 p_local) {
    vec3 q = abs(p_local) - vec3(0.5);
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0);
})";
    });
    sdf_rgy.register_c_sdf(cube_id, [](auto& entity, auto p) {
        glm::vec3 q = glm::abs(p) - glm::vec3(0.5f);
        return glm::length((glm::max)(q, 0.0f)) + (glm::min)((glm::max)(q.x, (glm::max)(q.y, q.z)), 0.0f);
    });
    auto torus_id = sdf_rgy.register_sdf("Torus", [](auto& shader, auto& constants) {
        return R"(float TorusSDF(vec3 p_local) {
    const vec2 unitTorusRadii = vec2(0.375, 0.125); // Major R, Minor r
    vec2 q = vec2(length(p_local.xz) - unitTorusRadii.x, p_local.y);
    return length(q) - unitTorusRadii.y;
})";
    });
    sdf_rgy.register_c_sdf(torus_id, [](auto& entity, auto p) {
        const glm::vec2 unitTorusRadii = glm::vec2(0.375f, 0.125f); // Major R, Minor r
        glm::vec2 q = glm::vec2(glm::length(glm::vec2(p.x, p.z)) - unitTorusRadii.x, p.y);
        return glm::length(q) - unitTorusRadii.y;
    });
    auto cylinder_id = sdf_rgy.register_sdf("Cylinder", [](auto& shader, auto& constants) {
        return R"(float CylinderSDF(vec3 p_local) {
    vec2 d = abs(vec2(length(p_local.xz), p_local.y)) - vec2(0.5, 0.5);
    return min(max(d.x, d.y), 0.0) + length(max(d, 0.0));
})";
    });
    sdf_rgy.register_c_sdf(cylinder_id, [](auto& entity, auto p) {
        glm::vec2 d = glm::abs(glm::vec2(glm::length(glm::vec2(p.x, p.z)), p.y)) - glm::vec2(0.5f, 0.5f);
        return (glm::min)((glm::max)(d.x, d.y), 0.0f) + glm::length((glm::max)(d, 0.0f));
    });
    auto cone_id = sdf_rgy.register_sdf("Cone", [](auto& shader, auto& constants) {
        return R"(float ConeSDF(vec3 p_local) {
    float r = 0.5; // Base radius
    float h = 1.0; // Height

    // Translate the local space down by 0.5
    vec3 p_adjusted = p_local + vec3(0.0, 0.5, 0.0);

    // Project the point onto the XZ plane and get the squared distance from the center
    float d_sq = p_adjusted.x * p_adjusted.x + p_adjusted.z * p_adjusted.z;

    // Squared radius at height y (linearly interpolating from r^2 at y=0 to 0 at y=h)
    float r_at_y_sq = r * r * (h - p_adjusted.y) * (h - p_adjusted.y) / (h * h);

    float signed_dist_surface = (r * p_adjusted.y + h * sqrt(d_sq) - r * h) / sqrt(r * r + h * h);

    if (p_adjusted.y > h) {
        return length(vec2(sqrt(d_sq), p_adjusted.y - h));
    }

    if (p_adjusted.y < 0.0) {
        return length(vec2(sqrt(d_sq), p_adjusted.y));
    }

    return signed_dist_surface;
})";
    });
    sdf_rgy.register_c_sdf(cone_id, [](auto& entity, auto p_local) {
        float r = 0.5f; // Base radius
        float h = 1.0f; // Height

        // Translate the local space down by 0.5
        glm::vec3 p_adjusted = p_local + glm::vec3(0.0f, 0.5f, 0.0f);

        // Project the point onto the XZ plane and get the squared distance from the center
        float d_sq = p_adjusted.x * p_adjusted.x + p_adjusted.z * p_adjusted.z;

        // Squared radius at height y (linearly interpolating from r^2 at y=0 to 0 at y=h)
        float r_at_y_sq = r * r * (h - p_adjusted.y) * (h - p_adjusted.y) / (h * h);

        float signed_dist_surface = (r * p_adjusted.y + h * glm::sqrt(d_sq) - r * h) / glm::sqrt(r * r + h * h);

        if (p_adjusted.y > h) {
            return glm::length(glm::vec2(glm::sqrt(d_sq), p_adjusted.y - h));
        }

        if (p_adjusted.y < 0.0) {
            return glm::length(glm::vec2(glm::sqrt(d_sq), p_adjusted.y));
        }

        return signed_dist_surface;
    });
    auto hexagonal_prism_id = sdf_rgy.register_sdf("HexagonalPrism", [](auto& shader, auto& constants) {
        return R"(float Hexagon2DSDF(vec2 p, float inradius_hex) {
    p = abs(p);
    // K_HEX_PRISM = vec3(-0.866025404, 0.5, 0.577350269)
    p -= 2.0 * min(dot(K_HEX_PRISM.xy, p), 0.0) * K_HEX_PRISM.xy;
    p -= vec2(clamp(p.x, -K_HEX_PRISM.z * inradius_hex, K_HEX_PRISM.z * inradius_hex), inradius_hex);
    return length(p) * sign(p.y);
}
float HexagonalPrismSDF(vec3 p_local) {
    float d_hex = Hexagon2DSDF(p_local.xz, SQRT3_DIV_4);
    float d_y = abs(p_local.y) - 0.5; // HalfHeight
    return length(max(vec2(d_hex, d_y), 0.0)) + min(max(d_hex, d_y), 0.0);
})";
    });
    sdf_rgy.register_c_sdf(hexagonal_prism_id, [](auto& entity, auto p) -> float {
        static constexpr glm::vec3 K_HEX_PRISM = glm::vec3(-0.866025404, 0.5, 0.577350269);
        static constexpr float SQRT3_DIV_4 = 0.43301270189; // sqrt(3)/4, inradius for unit hexagon with circumradius 0.5
        static auto Hexagon2DSDF = [](glm::vec2 p, float inradius_hex) -> float {
            p = glm::abs(p);
            p -= 2.0f * (glm::min)(glm::dot(glm::vec2(K_HEX_PRISM.x, K_HEX_PRISM.y), p), 0.0f) * glm::vec2(K_HEX_PRISM.x, K_HEX_PRISM.y);
            p -= glm::vec2(glm::clamp(p.x, -K_HEX_PRISM.z * inradius_hex, K_HEX_PRISM.z * inradius_hex), inradius_hex);
            return glm::length(p) * glm::sign(p.y);
        };
        float d_hex = Hexagon2DSDF(glm::vec2(p.x, p.z), SQRT3_DIV_4);
        float d_y = glm::abs(p.y) - 0.5f; // HalfHeight
        return glm::length((glm::max)(glm::vec2(d_hex, d_y), 0.0f)) + (glm::min)((glm::max)(d_hex, d_y), 0.0f);
    });
    auto rounded_cube_id = sdf_rgy.register_sdf("RoundedCube", [](auto& shader, auto& constants) {
        return R"(float RoundedCubeSDF(vec3 p_local) {
    float cornerRadius = 0.2;
    float effCornerRadius = min(cornerRadius, 0.5 - SDF_UNIT_EPSILON); // Cap radius
    vec3 q = abs(p_local) - vec3(0.5 - effCornerRadius);
    return length(max(q, 0.0)) + min(max(q.x, max(q.y, q.z)), 0.0) - effCornerRadius;
})";
    });
    sdf_rgy.register_c_sdf(rounded_cube_id, [](auto& entity, auto p) {
        float cornerRadius = 0.2f;
        float effCornerRadius = (glm::min)(cornerRadius, 0.5f); // Cap radius
        glm::vec3 q = glm::abs(p) - glm::vec3(0.5 - effCornerRadius);
        return glm::length((glm::max)(q, 0.0f)) + (glm::min)((glm::max)(q.x, (glm::max)(q.y, q.z)), 0.0f) - effCornerRadius;
    });
        // Ellipsoid SDF
    // Parameters: radii (rx, ry, rz)
    // This is an approximate distance for an axis-aligned ellipsoid, commonly used in raymarching.
    auto ellipsoid_id = sdf_rgy.register_sdf("Ellipsoid", [](auto& shader, auto& constants) {
        // Example radii - these would typically be passed via constants or entity properties
        // constants["ellipsoid_radii"] = glm::vec3(1.0f, 1.5f, 0.8f);
        return R"(float EllipsoidSDF(vec3 p_local) {
    // Example radii - ideally passed as a uniform or constant
    vec3 radii = vec3(1.0, 1.5, 0.8); // Example: stretched along Y, squashed along Z

    // Approximate distance to an axis-aligned ellipsoid
    // A more accurate SDF is complex, this is a common approximation for raymarching
    vec3 p_div_r = p_local / radii;
    float k0 = length(p_div_r);
    float k1 = length(p_div_r / (k0*k0)); // This term helps refine the approximation
    return k0 * (k0 - 1.0) / k1; // This formula provides a reasonable approximation
})";
    });
    sdf_rgy.register_c_sdf(ellipsoid_id, [](auto& entity, auto p) {
        // Example radii - these would typically be part of the entity's data
        glm::vec3 radii = glm::vec3(1.0f, 1.5f, 0.8f); // Example: stretched along Y, squashed along Z

        // Approximate distance calculation (matching the GLSL logic)
        glm::vec3 p_div_r = p / radii;
        float k0 = glm::length(p_div_r);
        float k1 = glm::length(p_div_r / (k0 * k0));
        return k0 * (k0 - 1.0f) / k1;
    });

    // Capsule SDF
    // Parameters: half_height (h), radius (r)
    // This capsule is aligned along the Y-axis, centered at the origin.
    auto capsule_id = sdf_rgy.register_sdf("Capsule", [](auto& shader, auto& constants) {
        // Example parameters - these would typically be passed via constants or entity properties
        // constants["capsule_half_height"] = 0.5f; // Half the height of the cylindrical part
        // constants["capsule_radius"] = 0.3f;     // Radius of the capsule
        return R"(float CapsuleSDF(vec3 p_local) {
    // Example parameters - ideally passed as uniforms or constants
    float half_height = 0.5; // Half the height of the cylindrical part
    float radius = 0.3;     // Radius of the capsule

    // Define the line segment for the cylinder part along the Y-axis
    vec3 segment_a = vec3(0.0, -half_height, 0.0);
    vec3 segment_b = vec3(0.0, half_height, 0.0);

    // Project the point p_local onto the line segment
    vec3 ba = segment_b - segment_a;
    float t = clamp(dot(p_local - segment_a, ba) / dot(ba, ba), 0.0, 1.0);

    // Find the closest point on the segment
    vec3 closest_point = segment_a + t * ba;

    // The distance is the distance from p_local to the closest point on the segment, minus the radius
    return length(p_local - closest_point) - radius;
})";
    });
    sdf_rgy.register_c_sdf(capsule_id, [](auto& entity, auto p) {
        // Example parameters - these would typically be part of the entity's data
        float half_height = 0.5f; // Half the height of the cylindrical part
        float radius = 0.3f;     // Radius of the capsule

        // Define the line segment for the cylinder part along the Y-axis
        glm::vec3 segment_a = glm::vec3(0.0f, -half_height, 0.0f);
        glm::vec3 segment_b = glm::vec3(0.0f, half_height, 0.0f);

        // Project the point p onto the line segment
        glm::vec3 ba = segment_b - segment_a;
        float t = glm::clamp(glm::dot(p - segment_a, ba) / glm::dot(ba, ba), 0.0f, 1.0f);

        // Find the closest point on the segment
        glm::vec3 closest_point = segment_a + t * ba;

        // The distance is the distance from p to the closest point on the segment, minus the radius
        return glm::length(p - closest_point) - radius;
    });
    auto flower_id = sdf_rgy.register_sdf("Flower", [](auto& shader, auto& constants) {
        // Example parameters - these would typically be passed via constants or entity properties
        // constants["flower_petal_count"] = 6.0f; // Number of petals
        // constants["flower_petal_power"] = 2.0f; // Controls the sharpness/roundness of petals
        // constants["flower_radius"] = 1.0f;      // Overall size of the flower
        return R"(float FlowerSDF(vec3 p_local) {
    // Example parameters - ideally passed as uniforms or constants
    float petal_count = 6.0; // Number of petals
    float petal_power = 2.0; // Controls the sharpness/roundness of petals
    float radius = 1.0;      // Overall size of the flower

    // Convert point to polar coordinates (ignoring Y for a 2D flower cross-section extruded along Y)
    // We'll work in the XZ plane for the flower shape
    float angle = atan(p_local.z, p_local.x); // Angle in the XZ plane
    float r_xz = length(p_local.xz);         // Distance in the XZ plane

    // Define the ideal radius for the flower shape at this angle
    // This uses cosine to create the petal pattern
    float ideal_radius = radius * (1.0 + 0.2 * cos(angle * petal_count)); // Adjust 0.2 for amplitude

    // Calculate the distance in the XZ plane relative to the ideal radius
    float dist_xz = r_xz - ideal_radius;

    // Combine with the Y-axis distance to make it a 3D shape (e.g., a flower extruded along Y)
    // A common way is to take the maximum of the 2D distance and the Y distance adjusted by a factor
    // This creates a shape that's a blend of the 2D flower and a cylinder/slab along Y.
    // For a more "rounded" 3D flower, you might use a smooth union or other combinations.
    // Let's use a simple combination for now.
    // We can also consider the distance from the Y axis directly for a different effect.
    // Let's try a simple distance based on the XZ plane and the Y coordinate.
    // This version treats it more like a 2D flower shape in the XZ plane, extended along Y.
    // We can refine this to be more 3D.

    // A better approach for a more 3D rounded flower:
    // Consider the distance from the origin, and modulate it based on the angle.
    float dist_from_origin = length(p_local);
    float angle_3d = atan(length(p_local.xz), p_local.y); // Angle from the Y-axis

    // Modulate the radius based on the angle in the XZ plane
    float modulated_radius = radius * (1.0 + 0.2 * cos(angle * petal_count));

    // Calculate the distance
    // This is a simplified approach. A more accurate 3D flower SDF is complex.
    // Let's stick to the 2D flower extruded along Y for simplicity and clarity with sin/cos.

    // Reverting to the 2D flower in XZ extruded along Y approach for clarity with sin/cos usage.
    // The distance is the distance in the XZ plane from the ideal radius, combined with the Y distance.
    // Use a smooth maximum or similar operation for a rounded 3D shape.
    // For simplicity, let's just use the 2D distance in XZ as the primary driver,
    // and perhaps add a constraint on the Y axis if needed for a capped shape.

    // Let's define the flower shape in 3D using spherical coordinates conceptually,
    // but implement using cartesian coordinates and trigonometric functions.
    // We can modulate the distance from the origin based on angles.

    // Angle in the XZ plane
    float theta = atan(p_local.z, p_local.x);
    // Angle from the Y-axis (polar angle)
    float phi = acos(p_local.y / length(p_local)); // Avoid division by zero near origin

    // Modulate the radius based on theta (for petals in XZ plane) and phi (for rounding in 3D)
    // Example modulation:
    float r_modulation = (1.0 + 0.2 * cos(theta * petal_count)) * (1.0 + 0.1 * cos(phi * petal_count)); // Combine modulations

    float ideal_r = radius * r_modulation;

    return length(p_local) - ideal_r;
})";
    });
    sdf_rgy.register_c_sdf(flower_id, [](auto& entity, auto p) {
        // Example parameters - these would typically be part of the entity's data
        float petal_count = 6.0f; // Number of petals
        float petal_power = 2.0f; // Controls the sharpness/roundness of petals (not directly used in this simple version)
        float radius = 1.0f;      // Overall size of the flower

        // Angle in the XZ plane
        float theta = atan2(p.z, p.x); // Use atan2 for correct angle over full circle
        // Angle from the Y-axis (polar angle)
        float r_p = glm::length(p);
        float phi = acos(p.y / (r_p + 1e-6f)); // Add small epsilon to avoid division by zero

        // Modulate the radius based on theta (for petals in XZ plane) and phi (for rounding in 3D)
        // Example modulation:
        float r_modulation = (1.0f + 0.2f * cos(theta * petal_count)) * (1.0f + 0.1f * cos(phi * petal_count)); // Combine modulations

        float ideal_r = radius * r_modulation;

        return r_p - ideal_r;
    });
    // WobblySphere SDF
    auto wobbly_sphere_id = sdf_rgy.register_sdf("WobblySphere", [](auto& shader, auto& constants) {
        return R"(
// WobblySphereSDF: A sphere with multi-frequency sinusoidal displacements.
// Creates a complex, round, and somewhat organic surface.
float WobblySphereSDF(vec3 p_local) {
    float base_radius = 0.4;
    float amplitude1 = 0.05;
    vec3 freq1 = vec3(5.0, 6.0, 7.0);
    float amplitude2 = 0.03;
    vec3 freq2 = vec3(11.0, 9.0, 13.0);

    float displacement = 0.0;
    // First layer of sinusoidal displacement
    displacement += amplitude1 * sin(p_local.x * freq1.x) * sin(p_local.y * freq1.y) * sin(p_local.z * freq1.z);
    // Second layer of cosinusoidal displacement with different frequencies and phase offsets
    displacement += amplitude2 * cos(p_local.x * freq2.x + 0.5) * cos(p_local.y * freq2.y + 1.0) * cos(p_local.z * freq2.z + 1.5);
    
    return length(p_local) - (base_radius + displacement);
}
)";
    });
    sdf_rgy.register_c_sdf(wobbly_sphere_id, [](auto& entity, auto p_local) {
        float base_radius = 0.4f;
        float amplitude1 = 0.05f;
        glm::vec3 freq1 = glm::vec3(5.0f, 6.0f, 7.0f);
        float amplitude2 = 0.03f;
        glm::vec3 freq2 = glm::vec3(11.0f, 9.0f, 13.0f);

        float displacement = 0.0f;
        // First layer of sinusoidal displacement
        displacement += amplitude1 * glm::sin(p_local.x * freq1.x) * glm::sin(p_local.y * freq1.y) * glm::sin(p_local.z * freq1.z);
        // Second layer of cosinusoidal displacement with different frequencies and phase offsets
        displacement += amplitude2 * glm::cos(p_local.x * freq2.x + 0.5f) * glm::cos(p_local.y * freq2.y + 1.0f) * glm::cos(p_local.z * freq2.z + 1.5f);
        
        return glm::length(p_local) - (base_radius + displacement);
    });

    // HelixoidDonut SDF
    auto helixoid_donut_id = sdf_rgy.register_sdf("HelixoidDonut", [](auto& shader, auto& constants) {
        return R"(
// HelixoidDonutSDF: A torus-like shape where the tube has a helical twist.
// R: Major radius of the torus
// r: Minor radius of the torus tube
// num_twists: Number of full twists of the tube around the major ring
float HelixoidDonutSDF(vec3 p_local) {
    float R = 0.35; 
    float r_minor = 0.1;  
    float num_twists = 4.0; 

    // Calculate distance from Y-axis for the main ring (cylindrical coordinate rho)
    float main_ring_dist_from_y_axis = length(p_local.xz);
    
    // q is the point in the 2D cross-section plane of the torus
    vec2 q = vec2(main_ring_dist_from_y_axis - R, p_local.y);

    // Angle around the Y-axis (azimuthal angle for the main ring)
    // GLSL's atan(y,x) is equivalent to C++ atan2(y,x)
    float angle_around_y = atan(p_local.z, p_local.x); 

    // Calculate the twist rotation
    float twist_angle = num_twists * angle_around_y;
    float c = cos(twist_angle);
    float s = sin(twist_angle);
    
    // Apply the twist rotation to the 2D cross-section point q
    // mat2 rot = mat2(c, -s, s, c);
    // vec2 twisted_q = rot * q;
    vec2 twisted_q = vec2(c * q.x - s * q.y, s * q.x + c * q.y);

    return length(twisted_q) - r_minor;
}
)";
    });
    sdf_rgy.register_c_sdf(helixoid_donut_id, [](auto& entity, auto p_local) {
        float R = 0.55f; // Major radius of the torus
        float r = 0.18f;  // Minor radius of the torus tube
        float num_twists = 8.0f; // Number of full twists of the tube around the major ring

        // Calculate distance from Y-axis for the main ring (cylindrical coordinate rho)
        float main_ring_dist_from_y_axis = glm::length(glm::vec2(p_local.x, p_local.z));
        
        // q is the point in the 2D cross-section plane of the torus
        glm::vec2 q = glm::vec2(main_ring_dist_from_y_axis - R, p_local.y);

        // Angle around the Y-axis (azimuthal angle for the main ring)
        // This angle determines the rotation for the twist
        float angle_around_y = glm::atan(p_local.z, p_local.x); // atan2(z,x)

        // Calculate the twist rotation
        float twist_angle = num_twists * angle_around_y;
        float c = glm::acos(twist_angle);
        float s = glm::asin(twist_angle);
        
        // Apply the twist rotation to the 2D cross-section point q
        glm::vec2 twisted_q = glm::vec2(c * q.y + s * q.x, s * q.y - c * q.x);

        return glm::length(twisted_q) - r;
    });

    // SphericalHarmonicsInspiredBlob SDF
    auto sh_blob_id = sdf_rgy.register_sdf("SphericalHarmonicsInspiredBlob", [](auto& shader, auto& constants) {
        return R"(
// SphericalHarmonicsInspiredBlobSDF: A blob-like shape whose radius is modulated
// by functions inspired by spherical harmonics. Creates organic, lobed forms.
float SphericalHarmonicsInspiredBlobSDF(vec3 p_local) {
    float base_radius = 0.25; // Adjusted base radius
    float r_len = length(p_local);

    // Handle singularity at the origin: consider it inside
    if (r_len < 0.00001) return -base_radius;

    // Convert to spherical coordinates
    // phi: polar angle (from +Z axis, 0 to PI)
    // theta: azimuthal angle (in XY plane from +X axis, -PI to PI)
    float phi = acos(clamp(p_local.z / r_len, -1.0, 1.0)); 
    float theta = atan(p_local.y, p_local.x); // GLSL atan(y,x)

    // Terms inspired by spherical harmonics to modulate the radius
    float term1 = 0.08 * cos(phi);                                  
    float term2 = 0.06 * sin(phi) * sin(phi) * cos(2.0 * theta);    
    float term3 = 0.04 * (3.0 * cos(phi) * cos(phi) - 1.0);         
    float term4 = 0.03 * pow(sin(phi), 3.0) * cos(3.0 * theta); 
    float term5 = 0.02 * cos(2.0 * phi) * sin(theta) * sin(phi);    

    float dynamic_radius = base_radius + term1 + term2 + term3 + term4 + term5;
    
    return r_len - dynamic_radius;
}
)";
    });
    sdf_rgy.register_c_sdf(sh_blob_id, [](auto& entity, auto p_local) {
        float base_radius = 0.25f; // Adjusted base radius
        float r_len = glm::length(p_local);

        // Handle singularity at the origin: consider it inside
        if (r_len < 0.00001f) return -base_radius;

        // Convert to spherical coordinates
        // phi: polar angle (from +Z axis), 0 to PI
        // theta: azimuthal angle (in XY plane from +X axis), -PI to PI
        float phi = glm::acos(glm::clamp(p_local.z / r_len, -1.0f, 1.0f));
        float theta = glm::atan(p_local.y, p_local.x); // atan2(y,x)

        // Terms inspired by spherical harmonics to modulate the radius
        // These create various lobes and indentations
        float term1 = 0.08f * glm::cos(phi);                                      // Elongates/compresses along Z (Y_1,0 like)
        float term2 = 0.06f * glm::sin(phi) * glm::sin(phi) * glm::cos(2.0f * theta); // 2 lobes in XY plane (Y_2,2 like)
        float term3 = 0.04f * (3.0f * glm::cos(phi) * glm::cos(phi) - 1.0f);     // Pinches/expands poles vs equator (Y_2,0 like)
        float term4 = 0.03f * glm::pow(glm::sin(phi), 3.0f) * glm::cos(3.0f * theta); // 3 lobes in XY, stronger at equator (Y_3,3 like)
        float term5 = 0.02f * glm::cos(2.0f * phi) * glm::sin(theta) * glm::sin(phi); // More complex polar/azimuthal interaction

        float dynamic_radius = base_radius + term1 + term2 + term3 + term4 + term5;
        
        return r_len - dynamic_radius;
    });
    auto box_frame_id = sdf_rgy.register_sdf("BoxFrame", [](auto& shader, auto& constants) -> std::string {
        return R"(float BoxFrameSDF(vec3 p) {
       p = abs(p  )-vec3(0.5, 0.5, 0.5);
  vec3 q = abs(p+0.1)-0.1;

  return min(min(
      length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
      length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
      length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
})";
    });
    sdf_rgy.register_c_sdf(box_frame_id, [](auto& entity, auto p) {
        p = glm::abs(p  )-glm::vec3(0.5f, 0.5f, 0.5f);
        glm::vec3 q = abs(p+0.1f)-0.1f;

        return (glm::min)((glm::min)(
            glm::length((glm::max)(glm::vec3(p.x,q.y,q.z),0.0f))+(glm::min)((glm::max)(p.x,(glm::max)(q.y,q.z)),0.0f),
            glm::length((glm::max)(glm::vec3(q.x,p.y,q.z),0.0f))+(glm::min)((glm::max)(q.x,(glm::max)(p.y,q.z)),0.0f)),
            glm::length((glm::max)(glm::vec3(q.x,q.y,p.z),0.0f))+(glm::min)((glm::max)(q.x,(glm::max)(q.y,p.z)),0.0f));
    });
    auto Horseshoe_id = sdf_rgy.register_sdf("Horseshoe", [](auto& shader, auto& constants) {
        return R"(float HorseshoeSDF(in vec3 p)
{
    vec2 c =  vec2(cos(1.3),sin(1.3));
    float r = 0.2;
    float le = 0.3;
    vec2 w = vec2(0.03,0.08);
    p.x = abs(p.x);
    float l = length(p.xy);
    p.xy = mat2(-c.x, c.y, 
              c.y, c.x)*p.xy;
    p.xy = vec2((p.y>0.0 || p.x>0.0)?p.x:l*sign(-c.x),
                (p.x>0.0)?p.y:l );
    p.xy = vec2(p.x,abs(p.y-r))-vec2(le,0.0);
    
    vec2 q = vec2(length(max(p.xy,0.0)) + min(0.0,max(p.x,p.y)),p.z);
    vec2 d = abs(q) - w;
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
})";
    });
    sdf_rgy.register_c_sdf(Horseshoe_id, [](auto& entity, auto p) {
        glm::vec2 c =  glm::vec2(cos(1.3f),sin(1.3f));
        float r = 0.2f;
        float le = 0.3f;
        glm::vec2 w = glm::vec2(0.03f,0.08f);
        p.x = abs(p.x);
        auto pxy = glm::vec2(p.x, p.y);
        float l = length(pxy);
        pxy = glm::mat2(-c.x, c.y, 
                c.y, c.x)*pxy;
        pxy = glm::vec2((pxy.y>0.0 || pxy.x>0.0)?pxy.x:l*glm::sign(-c.x),
                    (pxy.x>0.0)?pxy.y:l );
        pxy = glm::vec2(pxy.x,glm::abs(pxy.y-r))-glm::vec2(le,0.0);
        glm::vec2 q = glm::vec2(glm::length((glm::max)(pxy,0.0f)) + (glm::min)(0.0f,(glm::max)(pxy.x,pxy.y)),p.z);
        glm::vec2 d = (glm::abs)(q) - w;
        return (glm::min)((glm::max)(d.x,d.y),0.0f) + glm::length((glm::max)(d,0.0f));
    });
    auto sine_wave_plane_id = sdf_rgy.register_sdf("SineWavePlane", [](auto& shader, auto& constants) {
        return R"(float SineWavePlaneSDF(vec3 p_local) {
    float amplitude = 0.1;
    float frequency = 10.0;
    return p_local.y - amplitude * sin(p_local.x * frequency);
})";
    });
    sdf_rgy.register_c_sdf(sine_wave_plane_id, [](auto& entity, auto p) {
        float amplitude = 0.1f;
        float frequency = 10.0f;
        return p.y - amplitude * glm::sin(p.x * frequency);
    });

    auto twisted_box_id = sdf_rgy.register_sdf("TwistedBox", [](auto& shader, auto& constants) {
        return R"(float TwistedBoxSDF(vec3 p_local) {
    float twist_amount = 5.0; // Adjust for more or less twist
    float c = cos(twist_amount * p_local.y);
    float s = sin(twist_amount * p_local.y);
    vec3 twisted_p = vec3(p_local.x * c - p_local.z * s, p_local.y, p_local.x * s + p_local.z * c);
    vec3 b = vec3(0.5); // Example size
    vec3 q = abs(twisted_p) - b;
    return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);
})";
    });
    sdf_rgy.register_c_sdf(twisted_box_id, [](auto& entity, auto p) {
        float twist_amount = 5.0f;
        float c = glm::cos(twist_amount * p.y);
        float s = glm::sin(twist_amount * p.y);
        glm::vec3 twisted_p = glm::vec3(p.x * c - p.z * s, p.y, p.x * s + p.z * c);
        glm::vec3 b = glm::vec3(0.5f);
        glm::vec3 q = glm::abs(twisted_p) - b;
        return glm::length((glm::max)(q, 0.0f)) + (glm::min)((glm::max)(q.x, (glm::max)(q.y, q.z)), 0.0f);
    });

    auto csg_sine_sphere_id = sdf_rgy.register_sdf("CSGSineSphere", [](auto& shader, auto& constants) {
        return R"(float CSGSineSphereSDF(vec3 p_local) {
    float r = 0.5;
    float sphere_dist = length(p_local) - r;

    // Use sine to create a "wave" and subtract it from the sphere
    float wave_dist = 0.1 * sin(p_local.x * 15.0) + 0.1 * cos(p_local.z * 15.0);

    return max(sphere_dist, -wave_dist); // Intersection for interesting cuts
})";
    });
    sdf_rgy.register_c_sdf(csg_sine_sphere_id, [](auto& entity, auto p) {
        float r = 0.5f;
        float sphere_dist = glm::length(p) - r;
        float wave_dist = 0.1f * glm::sin(p.x * 15.0f) + 0.1f * glm::cos(p.z * 15.0f);
        return (glm::max)(sphere_dist, -wave_dist);
    });

    auto tangent_cone_id = sdf_rgy.register_sdf("TangentCone", [](auto& shader, auto& constants) {
        return R"(float TangentConeSDF(vec3 p_local) {
    // This is a simplified cone that uses tangent for its angle, showing an example usage
    // A more robust cone SDF would use a dot product, but this demonstrates tangent
    float angle = 0.8; // Controls the cone's "steepness"
    float d = length(p_local.xz) * tan(angle) - p_local.y;
    float r_cap = 0.5; // Radius of the base
    float h = 1.0; // Height of the cone
    
    // Combining cone with plane caps
    float cone_dist = max(d, -p_local.y); // Infinite cone base is p_local.y = 0
    cone_dist = max(cone_dist, p_local.y - h); // Top cap
    
    // Smooth transition near the tip and base for visual effect (optional)
    return cone_dist;
})";
    });
    sdf_rgy.register_c_sdf(tangent_cone_id, [](auto& entity, auto p) {
        float angle = 0.8f;
        float d = glm::length(glm::vec2(p.x, p.z)) * glm::tan(angle) - p.y;
        float r_cap = 0.5f;
        float h = 1.0f;
        float cone_dist = (glm::max)(d, -p.y);
        cone_dist = (glm::max)(cone_dist, p.y - h);
        return cone_dist;
    });
    auto menger_sponge_iter_id = sdf_rgy.register_sdf("MengerSpongeIter", [](auto& shader, auto& constants) {
        return R"(float MengerSpongeIterSDF(vec3 p_local) {
    vec3 p = p_local;
    float d = length(p) - 1.0; // Start with a sphere or large box to enclose

    for (int i = 0; i < 3; ++i) { // Number of iterations, adjust for complexity vs. performance
        p = abs(p);
        if (p.x < p.y) p.xy = p.yx;
        if (p.x < p.z) p.xz = p.zx;
        if (p.y < p.z) p.yz = p.zy;
        
        p = p * 3.0 - 1.0; // Scale and translate
        
        float da = max(abs(p.x), abs(p.y));
        float db = max(da, abs(p.z));
        float dc = dot(p, normalize(vec3(1.0))); // Optional: A different subtraction
        
        // This is a simplified Menger Sponge iteration logic
        // Subtracting middle parts of each dimension
        d = max(d, -db); // Remove central part for a box
    }
    
    vec3 b = vec3(0.5); // Bounding box for visual effect, optional
    vec3 q = abs(p_local) - b;
    float box_dist = length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0);

    return max(d, box_dist); // Combine the fractal logic with a bounding box/sphere
})";
    });
    sdf_rgy.register_c_sdf(menger_sponge_iter_id, [](auto& entity, auto p_local) {
        glm::vec3 p = p_local;
        float d = glm::length(p) - 1.0f; // Start with a sphere

        for (int i = 0; i < 3; ++i) {
            p = glm::abs(p);
            // Sorting components for specific fractal types
            if (p.x < p.y) std::swap(p.x, p.y);
            if (p.x < p.z) std::swap(p.x, p.z);
            if (p.y < p.z) std::swap(p.y, p.z);
            
            p = p * 3.0f - 1.0f;
            
            float da = (glm::max)(glm::abs(p.x), glm::abs(p.y));
            float db = (glm::max)(da, glm::abs(p.z));
            
            d = (glm::max)(d, -db);
        }
        
        glm::vec3 b = glm::vec3(0.5f);
        glm::vec3 q = glm::abs(p_local) - b;
        float box_dist = glm::length((glm::max)(q, 0.0f)) + (glm::min)((glm::max)(q.x, (glm::max)(q.y, q.z)), 0.0f);

        return (glm::max)(d, box_dist);
    });

    auto heart_shape_id = sdf_rgy.register_sdf("HeartShape", [](auto& shader, auto& constants) {
        return R"(float HeartShapeSDF(vec3 p_local) {
    vec2 p = p_local.xy;
    p.x = abs(p.x);
    float a = p.x - 0.5;
    float b = p.y + 0.5;
    float c = 1.0 - p.x - p.y;
    
    // A classic heart shape formula, typically 2D, extended to 3D via extrusion or revolve
    // This particular formula for heart is generally polynomial based
    float f = pow(p.x*p.x + p.y*p.y - 1.0, 3.0) - p.x*p.x*p.y*p.y*p.y; // 2D Heart implicit
    
    // Extrude the 2D heart along Z for 3D SDF
    float thickness = 0.2;
    float dist2D = -f; // Invert for outside positive
    
    vec2 q = vec2(dist2D, abs(p_local.z) - thickness);
    return length(max(q,0.0)) + min(max(q.x,q.y),0.0);
})";
    });
    sdf_rgy.register_c_sdf(heart_shape_id, [](auto& entity, auto p_local) {
        glm::vec2 p = glm::vec2(p_local.x, p_local.y);
        p.x = glm::abs(p.x);
        
        float f = glm::pow(glm::pow(p.x, 2.0f) + glm::pow(p.y, 2.0f) - 1.0f, 3.0f) - glm::pow(p.x, 2.0f) * glm::pow(p.y, 3.0f);
        
        float thickness = 0.2f;
        float dist2D = -f;
        
        glm::vec2 q = glm::vec2(dist2D, glm::abs(p_local.z) - thickness);
        return glm::length((glm::max)(q, 0.0f)) + (glm::min)((glm::max)(q.x, q.y), 0.0f);
    });

    auto wavy_capsule_id = sdf_rgy.register_sdf("WavyCapsule", [](auto& shader, auto& constants) {
        return R"(float WavyCapsuleSDF(vec3 p_local) {
    vec3 a = vec3(0.0, -0.5, 0.0); // Start point of capsule segment
    vec3 b = vec3(0.0, 0.5, 0.0);  // End point of capsule segment
    float r = 0.3; // Radius of capsule

    vec3 pa = p_local - a;
    vec3 ba = b - a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    float capsule_dist = length( pa - ba*h ) - r;

    // Add a wavy deformation based on x and z
    float wave_amplitude = 0.1;
    float wave_frequency = 15.0;
    float wavy_offset = wave_amplitude * (sin(p_local.x * wave_frequency) + cos(p_local.z * wave_frequency));
    
    // Apply the wave only to the distance, creating an interesting surface
    return capsule_dist - wavy_offset;
})";
    });
    sdf_rgy.register_c_sdf(wavy_capsule_id, [](auto& entity, auto p_local) {
        glm::vec3 a = glm::vec3(0.0f, -0.5f, 0.0f);
        glm::vec3 b = glm::vec3(0.0f, 0.5f, 0.0f);
        float r = 0.3f;

        glm::vec3 pa = p_local - a;
        glm::vec3 ba = b - a;
        float h = glm::clamp( glm::dot(pa, ba) / glm::dot(ba, ba), 0.0f, 1.0f );
        float capsule_dist = glm::length( pa - ba * h ) - r;

        float wave_amplitude = 0.1f;
        float wave_frequency = 15.0f;
        float wavy_offset = wave_amplitude * (glm::sin(p_local.x * wave_frequency) + glm::cos(p_local.z * wave_frequency));
        
        return capsule_dist - wavy_offset;
    });

    auto cross_shape_id = sdf_rgy.register_sdf("CrossShape", [](auto& shader, auto& constants) {
        return R"(float CrossShapeSDF(vec3 p_local) {
    vec3 p = abs(p_local);
    vec3 b = vec3(0.2, 0.2, 0.8); // Dimensions of arms

    // SDF for 3 intersecting boxes (a cross)
    float d1 = length(max(abs(p) - b.zyx, 0.0)) + min(0.0, max(abs(p.x) - b.z, max(abs(p.y) - b.y, abs(p.z) - b.x)));
    float d2 = length(max(abs(p) - b.xzy, 0.0)) + min(0.0, max(abs(p.x) - b.x, max(abs(p.y) - b.z, abs(p.z) - b.y)));
    float d3 = length(max(abs(p) - b.yxz, 0.0)) + min(0.0, max(abs(p.x) - b.y, max(abs(p.y) - b.x, abs(p.z) - b.z)));

    return min(min(d1, d2), d3);
})";
    });
    sdf_rgy.register_c_sdf(cross_shape_id, [](auto& entity, auto p_local) {
        glm::vec3 p = glm::abs(p_local);
        glm::vec3 b = glm::vec3(0.2f, 0.2f, 0.8f); // Dimensions of arms

        float d1 = glm::length((glm::max)(glm::abs(p) - glm::vec3(b.z, b.y, b.x), 0.0f)) + (glm::min)(0.0f, (glm::max)(glm::abs(p.x) - b.z, (glm::max)(glm::abs(p.y) - b.y, glm::abs(p.z) - b.x)));
        float d2 = glm::length((glm::max)(glm::abs(p) - glm::vec3(b.x, b.z, b.y), 0.0f)) + (glm::min)(0.0f, (glm::max)(glm::abs(p.x) - b.x, (glm::max)(glm::abs(p.y) - b.z, glm::abs(p.z) - b.y)));
        float d3 = glm::length((glm::max)(glm::abs(p) - glm::vec3(b.y, b.x, b.z), 0.0f)) + (glm::min)(0.0f, (glm::max)(glm::abs(p.x) - b.y, (glm::max)(glm::abs(p.y) - b.x, glm::abs(p.z) - b.z)));

        return glm::min(glm::min(d1, d2), d3);
    });
    registered_zg_sdfs =  true;
}