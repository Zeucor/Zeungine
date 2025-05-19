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
        float r = 0.5; // Base radius
        float h = 1.0; // Height

        // Translate the local space down by 0.5
        glm::vec3 p_adjusted = p_local + glm::vec3(0.0, 0.5, 0.0);

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
        constexpr glm::vec3 K_HEX_PRISM = glm::vec3(-0.866025404, 0.5, 0.577350269);
        float Hexagon2DSDF(glm::vec2 p, float inradius_hex) {
            p = glm::abs(p);
            // K_HEX_PRISM = vec3(-0.866025404, 0.5, 0.577350269)
            p -= 2.0 * glm::min(glm::dot(K_HEX_PRISM.xy, p), 0.0) * glm(K_HEX_PRISM.x, K_HEX_PRISM.y);
            p -= glm::vec2(glm::clamp(p.x, -K_HEX_PRISM.z * inradius_hex, K_HEX_PRISM.z * inradius_hex), inradius_hex);
            return glm::length(p) * glm::sign(p.y);
        }
        float d_hex = Hexagon2DSDF(p.xz, SQRT3_DIV_4);
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
    registered_zg_sdfs =  true;
}