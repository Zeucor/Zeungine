#include <zg/Registry.hpp>
#include <zg/components/scenes/SMAA.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/Window.hpp>
using namespace zg;
using namespace zg::components::scenes;
zg::components::scenes::SceneComponentCreateInfo zg::components::scenes::SMAAFactory(
    float threshold,
    float maxSearchSteps,
    float maxSearchStepsDiag,
    float cornerRounding
)
{
	SceneComponentCreateInfo info{
        .name = "SMAA",
        .onAttachedFunction = [
            threshold,
            maxSearchSteps,
            maxSearchStepsDiag,
            cornerRounding
        ](auto& component)
        {
            auto& scene = Registry::getScene(component.HOST_INDEX_STACK);
            PostProcessingStageCreateInfo smaaStageCreateInfo{
                .name = "SMAA",
                .inputs = {"ColorTexture"},
                .outputs = {{"ColorTexture", textures::Framebuffer::AttachmentType::Color, textures::Texture::AddressMode::ClampToEdge}},
                .constants = {"SMAA"},
                .setShaderConstants = [
                    threshold,
                    maxSearchSteps,
                    maxSearchStepsDiag,
                    cornerRounding
                ](shaders::Shader& shader, auto& vao)
                {
                    float values[4] = {
                        threshold,
                        maxSearchSteps,
                        maxSearchStepsDiag,
                        cornerRounding
                    };
                    shader.setBlock("SMAAValues", vao, values);
                    glm::vec2 inverseScreenSize(1.f / shader.iRenderer->platformWindowPointer->renderWindowPointer->windowWidth, 1.f / shader.iRenderer->platformWindowPointer->renderWindowPointer->windowHeight);
                    shader.setBlock("InverseScreenSize", vao, inverseScreenSize);
                },
                .staticOnAttached = []()
                {
                    shaders::ShaderFactory::addHook(
                        shaders::ShaderType::Fragment,
                        "layout",
                        "SMAA",
                        [](shaders::Shader& shader, const auto& constants) -> std::string {
                            uint32_t bindingIndex = 0;
                            bindingIndex = shaders::ShaderFactory::currentBindingIndex++;
                            shader.addUBO(shaders::ShaderType::Fragment, "SMAAValues", bindingIndex, sizeof(float) * 4);
                            std::string string =
                                "layout(binding = " + std::to_string(bindingIndex) + ") uniform SMAAValues {\n" +
                                "  float threshold;\n" +
                                "  float maxSearchSteps;\n" +
                                "  float maxSearchStepsDiag;\n" +
                                "  float cornerRounding;\n" +
                                "} smaaValues;\n";
                            bindingIndex = shaders::ShaderFactory::currentBindingIndex++;
                            shader.addUBO(shaders::ShaderType::Fragment, "InverseScreenSize", bindingIndex, sizeof(glm::vec2));
                            string +=
                                "layout(binding = " + std::to_string(bindingIndex) + ") uniform InverseScreenSize {\n" +
                                "  vec2 size;\n" +
                                "} inverseScreenSize;\n";
                            string += "#define SMAA_TEXTURE_FETCH(tex, coord) texture(tex, coord)\n"
                                      "#define SMAA_TEXTURE_FETCH_OFFSET(tex, coord, offset) textureOffset(tex, coord, offset)\n"
                                      "float Luminance(vec3 color);\n"
                                      "vec4 SMAAColorEdgeDetectionPS(vec2 texcoord);\n"
                                      "float SMAASearchLength(vec2 texcoord, vec2 direction);\n"
                                      "vec2 SMAASearchDiag(vec2 texcoord, vec2 dir, out vec2 e);\n"
                                      "vec2 SMAAArea(vec2 dist, float e1, float e2);\n"
                                      "vec4 SMAABlendingWeightCalculationPS(vec2 texcoord, vec4 edges);\n"
                                      "vec4 SMAANeighborhoodBlendingPS(vec2 texcoord, vec4 weights);\n";
                            return string;
                        }
                    );
                    shaders::ShaderFactory::addHook(
                        shaders::ShaderType::Fragment,
                        "postPostInMain",
                        "SMAA",
                        [](auto& shader, const auto& constants) -> std::string {
                            return "vec4 edges = SMAAColorEdgeDetectionPS(inUV);\n"
                                    "if (dot(edges, edges) < 0.01) {\n"
                                    "    FragColor = SMAA_TEXTURE_FETCH(ColorTexture, inUV);\n"
                                    "    return;\n"
                                    "}\n"
                                    "vec4 weights = SMAABlendingWeightCalculationPS(inUV, edges);\n"
                                    "FragColor = SMAANeighborhoodBlendingPS(inUV, weights);";
                        }
                    );
                    shaders::ShaderFactory::addHook(
                        shaders::ShaderType::Fragment,
                        "postMain",
                        "SMAA",
                        [](auto& shader, const auto& constants) -> std::string {
                            return R"(
float Luminance(vec3 color) {
    return dot(color, vec3(0.2126, 0.7152, 0.0722));
}
vec4 SMAAColorEdgeDetectionPS(vec2 texcoord) {
    vec2 pixelSize = inverseScreenSize.size;
    vec3 colorCenter = SMAA_TEXTURE_FETCH(ColorTexture, texcoord).rgb;
    vec3 colorTop    = SMAA_TEXTURE_FETCH(ColorTexture, texcoord + vec2(0.0, pixelSize.y)).rgb;
    vec3 colorBottom = SMAA_TEXTURE_FETCH(ColorTexture, texcoord + vec2(0.0, -pixelSize.y)).rgb;
    vec3 colorLeft   = SMAA_TEXTURE_FETCH(ColorTexture, texcoord + vec2(-pixelSize.x, 0.0)).rgb;
    vec3 colorRight  = SMAA_TEXTURE_FETCH(ColorTexture, texcoord + vec2(pixelSize.x, 0.0)).rgb;
    float lumaCenter = Luminance(colorCenter);
    float lumaTop    = Luminance(colorTop);
    float lumaBottom = Luminance(colorBottom);
    float lumaLeft   = Luminance(colorLeft);
    float lumaRight  = Luminance(colorRight);
    float deltaLuminanceT = abs(lumaCenter - lumaTop);
    float deltaLuminanceB = abs(lumaCenter - lumaBottom);
    float deltaLuminanceL = abs(lumaCenter - lumaLeft);
    float deltaLuminanceR = abs(lumaCenter - lumaRight);
    vec4 edges = vec4(0.0);
    edges.x = step(smaaValues.threshold, deltaLuminanceT); // Top edge
    edges.y = step(smaaValues.threshold, deltaLuminanceB); // Bottom edge
    edges.z = step(smaaValues.threshold, deltaLuminanceL); // Left edge
    edges.w = step(smaaValues.threshold, deltaLuminanceR); // Right edge
    if (dot(edges, edges) == 0.0) {
        return vec4(0.0);
    }
    return edges;
}
float SMAASearchLength(vec2 texcoord, vec2 direction) {
    vec2 pixelSize = inverseScreenSize.size;
    float steps = 0.0;
    vec2 currentCoord = texcoord + direction * pixelSize;
    for (int i = 0; i < smaaValues.maxSearchSteps; i++) {
        if (dot(step(vec2(0.0), currentCoord), step(currentCoord, vec2(1.0))) < 2.0) break;
        vec4 edges = SMAAColorEdgeDetectionPS(currentCoord);
        float edgeValue = 0.0;
        if (direction.x != 0.0) { // Horizontal search
            edgeValue = (direction.x < 0.0) ? edges.z : edges.w; // Check Left or Right edge flag
        } else { // Vertical search
            edgeValue = (direction.y < 0.0) ? edges.y : edges.x; // Check Bottom or Top edge flag
        }
        if (edgeValue < 0.5) break; // Edge ended
        steps += 1.0;
        currentCoord += direction * pixelSize;
    }
    return steps;
}
vec2 SMAASearchDiag(vec2 texcoord, vec2 dir, out vec2 e) {
    vec2 pixelSize = inverseScreenSize.size;
    e = vec2(0.0);
    vec2 currentCoord = texcoord + dir * pixelSize;
    for (int i = 0; i < smaaValues.maxSearchStepsDiag; i++) {
        if (dot(step(vec2(0.0), currentCoord), step(currentCoord, vec2(1.0))) < 2.0) break;
        vec4 edges = SMAAColorEdgeDetectionPS(currentCoord);
        // Determine the relevant edge component based on direction
        float edgeValue = 0.0;
        if (dir.x != 0.0) { // Searching horizontally first (e.g., TR, BR)
            edgeValue = (dir.x < 0.0) ? edges.z : edges.w;
        } else { // Searching vertically first (e.g., TL, BL) - Should not happen with standard diag search?
            edgeValue = (dir.y < 0.0) ? edges.y : edges.x; // Adjust if needed
        }
        // Check for crossing edge (perpendicular to search direction)
        float crossEdgeValue = 0.0;
            if (dir.y != 0.0) { // Check vertical edge if moving vertically
            crossEdgeValue = (dir.y < 0.0) ? edges.y : edges.x;
        } else { // Check horizontal edge if moving horizontally
                crossEdgeValue = (dir.x < 0.0) ? edges.z : edges.w;
        }
        // Store crossing edge offset
        e.y = crossEdgeValue;
        // If edge continues and no crossing edge is found yet
        if (edgeValue > 0.5 && crossEdgeValue < 0.5) {
                e.x = i + 1; // Store distance
                currentCoord += dir * pixelSize;
        } else {
            break; // Edge ended or crossed
        }
    }
    return e;
}
vec2 SMAAArea(vec2 dist, float e1, float e2) {
    vec2 pixcoord = vec2(dist.x, 1.0 - dist.y); // Map distances to pseudo-texture coords
    vec2 weights = pixcoord * pixcoord * vec2(0.5, 0.5); // Basic quadratic falloff
    // Reduce weight if the edge ends abruptly (simulates corner detection)
    weights *= smoothstep(0.75, 1.0, vec2(e1, e2));
    return weights;
}
vec4 SMAABlendingWeightCalculationPS(vec2 texcoord, vec4 edges) {
    vec4 weights = vec4(0.0); // Final weights (left, right, top, bottom)
    // --- Horizontal Edge Processing ---
    if (edges.z > 0.5 || edges.w > 0.5) { // If Left or Right edge exists
        vec2 pixelSize = inverseScreenSize.size;
        // Search Left and Right
        float leftDist = SMAASearchLength(texcoord, vec2(-1.0, 0.0));
        float rightDist = SMAASearchLength(texcoord, vec2(1.0, 0.0));
        // Calculate horizontal offset
        float totalDist = leftDist + rightDist;
        float offset = 0.0;
        if (totalDist > 0.0) {
            offset = (rightDist) / totalDist; // Normalized offset [0, 1]
        }
        // --- Diagonal Searches for Crossing Edges ---
        vec2 e_tl, e_bl; // Top-Left, Bottom-Left diagonal search results
        vec2 diagTL = SMAASearchDiag(texcoord, vec2(-1.0, 1.0), e_tl);  // Search Top-Left
        vec2 diagBL = SMAASearchDiag(texcoord, vec2(-1.0, -1.0), e_bl); // Search Bottom-Left
        vec2 e_tr, e_br; // Top-Right, Bottom-Right diagonal search results
        vec2 diagTR = SMAASearchDiag(texcoord, vec2(1.0, 1.0), e_tr);   // Search Top-Right
        vec2 diagBR = SMAASearchDiag(texcoord, vec2(1.0, -1.0), e_br);  // Search Bottom-Right
        // --- Calculate Weights based on Patterns (Approximation) ---
        // This part simulates the logic that uses AreaTex and SearchTex
        // Calculate basic weight based on distance (similar to simplified version)
        float baseWeight = smoothstep(0.0, 1.0, totalDist / float(smaaValues.maxSearchSteps * 2));
        // Adjust weight for corners (using diagonal search results)
        float cornerFactor = 1.0;
        // Example: Reduce weight if a crossing edge is found nearby diagonally
        cornerFactor *= (1.0 - 0.5 * max(e_tl.y, e_bl.y)); // Left side corners
        cornerFactor *= (1.0 - 0.5 * max(e_tr.y, e_br.y)); // Right side corners
        // Apply corner rounding factor (more complex logic needed for precise rounding)
        float rounding = 1.0 - float(smaaValues.cornerRounding) / 100.0;
        cornerFactor = mix(1.0, cornerFactor, rounding);
        // Calculate final horizontal weights (distribute based on offset)
        weights.z = baseWeight * cornerFactor * (1.0 - offset); // Left weight
        weights.w = baseWeight * cornerFactor * offset;         // Right weight
    }
    // --- Vertical Edge Processing (Similar logic) ---
    if (edges.x > 0.5 || edges.y > 0.5) { // If Top or Bottom edge exists
        vec2 pixelSize = inverseScreenSize.size;
        // Search Top and Bottom
        float topDist = SMAASearchLength(texcoord, vec2(0.0, 1.0));
        float bottomDist = SMAASearchLength(texcoord, vec2(0.0, -1.0));
        // Calculate vertical offset
        float totalDist = topDist + bottomDist;
        float offset = 0.0;
        if (totalDist > 0.0) {
            offset = (topDist) / totalDist; // Normalized offset [0, 1]
        }
        // --- Diagonal Searches (already done, reuse results if possible or re-calc if needed) ---
        // We need diagonal results relative to vertical search now.
        // Note: The diagonal search logic might need adjustment depending on how
        // edge continuation vs. crossing is defined relative to the primary search axis.
        // For simplicity, we'll assume the previous diagonal searches give relevant info.
        // --- Calculate Weights based on Patterns (Approximation) ---
        float baseWeight = smoothstep(0.0, 1.0, totalDist / float(smaaValues.maxSearchSteps * 2));
        float cornerFactor = 1.0;
        vec2 e_tl, e_bl; // Top-Left, Bottom-Left diagonal search results
        vec2 diagTL = SMAASearchDiag(texcoord, vec2(-1.0, 1.0), e_tl);  // Search Top-Left
        vec2 diagBL = SMAASearchDiag(texcoord, vec2(-1.0, -1.0), e_bl); // Search Bottom-Left
        vec2 e_tr, e_br; // Top-Right, Bottom-Right diagonal search results
        vec2 diagTR = SMAASearchDiag(texcoord, vec2(1.0, 1.0), e_tr);   // Search Top-Right
        vec2 diagBR = SMAASearchDiag(texcoord, vec2(1.0, -1.0), e_br);  // Search Bottom-Right
        cornerFactor *= (1.0 - 0.5 * max(e_tl.y, e_tr.y)); // Top side corners
        cornerFactor *= (1.0 - 0.5 * max(e_bl.y, e_br.y)); // Bottom side corners
        float rounding = 1.0 - float(smaaValues.cornerRounding) / 100.0;
        cornerFactor = mix(1.0, cornerFactor, rounding);
        // Calculate final vertical weights
        weights.x = baseWeight * cornerFactor * offset;         // Top weight
        weights.y = baseWeight * cornerFactor * (1.0 - offset); // Bottom weight
    }
    return weights;
}
// Neighborhood Blending Function
// Blends the center pixel with its neighbors based on the calculated weights.
vec4 SMAANeighborhoodBlendingPS(vec2 texcoord, vec4 weights) {
    vec2 pixelSize = inverseScreenSize.size;
    // Sample neighbors
    vec4 colorCenter = SMAA_TEXTURE_FETCH(ColorTexture, texcoord);
    vec4 colorTop    = SMAA_TEXTURE_FETCH(ColorTexture, texcoord + vec2(0.0, pixelSize.y));
    vec4 colorBottom = SMAA_TEXTURE_FETCH(ColorTexture, texcoord + vec2(0.0, -pixelSize.y));
    vec4 colorLeft   = SMAA_TEXTURE_FETCH(ColorTexture, texcoord + vec2(-pixelSize.x, 0.0));
    vec4 colorRight  = SMAA_TEXTURE_FETCH(ColorTexture, texcoord + vec2(pixelSize.x, 0.0));
    // Blend contributions
    vec4 blendedColor = colorCenter;
    blendedColor = mix(blendedColor, colorTop,    weights.x); // Blend top
    blendedColor = mix(blendedColor, colorBottom, weights.y); // Blend bottom
    blendedColor = mix(blendedColor, colorLeft,   weights.z); // Blend left
    blendedColor = mix(blendedColor, colorRight,  weights.w); // Blend right
    // Normalize weight sum (important if weights don't sum to 1)
    // float totalWeight = dot(weights, vec4(1.0));
    // if (totalWeight > 0.0) {
    //     // Complex normalization might be needed depending on how weights are calculated
    //     // For simple mix, it might not be strictly necessary if weights are treated as alpha
    // }
    // Simplified approach: Assume mix handles the weighting distribution.
    // The sum of weights might exceed 1.0 in areas with both H and V edges.
    // A more correct blend might involve calculating a weighted average explicitly.
    // Explicit weighted average attempt:
    float weightSum = weights.x + weights.y + weights.z + weights.w;
    if (weightSum > 0.01) { // Only blend if there are weights
            blendedColor = (colorCenter * (1.0 - weightSum) + // Center contribution diminished by total weight
                            colorTop    * weights.x +
                            colorBottom * weights.y +
                            colorLeft   * weights.z +
                            colorRight  * weights.w);
            // Clamp center weight contribution to avoid negative values if weightSum > 1
            float centerWeight = max(0.0, 1.0 - weightSum);
            blendedColor = (colorCenter * centerWeight +
                            colorTop    * weights.x +
                            colorBottom * weights.y +
                            colorLeft   * weights.z +
                            colorRight  * weights.w) / (centerWeight + weightSum); // Normalize
    } else {
        blendedColor = colorCenter; // No edges, return original
    }
    return blendedColor;
})";
                        }
                    );
                }
            };
            scene.postProcessingPipeline.addStage(100.f, smaaStageCreateInfo);
        }
    };
	return info;
}
