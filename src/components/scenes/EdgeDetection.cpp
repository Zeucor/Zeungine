#include <zg/Registry.hpp>
#include <zg/components/scenes/EdgeDetection.hpp>
#include <zg/shaders/Shader.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/Window.hpp>
#include <glm/glm.hpp> // Ensure glm is included

using namespace zg;
using namespace zg::components::scenes;
using zg::Registry;
using zg::shaders::Shader;
using zg::shaders::ShaderFactory;
using zg::shaders::ShaderType;

SceneComponentCreateInfo components::scenes::EdgeDetectionFactory() {
    SceneComponentCreateInfo info{
        .name = "EdgeDetection",
        .onAttachedFunction = [](auto& component) {
            auto& scene = Registry::GetSingleton().getScene(component.HOST_INDEX_STACK);
            component.template make<glm::vec4>("edgeColor", 0.0f, 0.0f, 0.0f, 1.0f);
            component.template make<glm::vec4>("backgroundColor", 1.0f, 1.0f, 1.0f, 0.0f);
            component.template make<float>("combinedThreshold", 0.3f);
            component.template make<float>("lineThickness", 0.5f);
            component.template make<float>("internal_colorSensitivity", 0.38f);
            // Adjusted depth sensitivity for Laplacian - might need tuning
            component.template make<float>("internal_depthSensitivity", 0.1f);


            PostProcessingStageCreateInfo edgeDetectionStageCreateInfo{
                .name = "EdgeDetection",
                .inputs = {"ColorTexture", "DepthTexture"},
                .outputs = {{"ColorTexture", textures::Framebuffer::AttachmentType::Color, textures::Texture::AddressMode::ClampToEdge}},
                .constants = {"EdgeDetection"},
                .setShaderConstants = [
                    HOST_INDEX_STACK = component.HOST_INDEX_STACK,
                    componentID = component.ID
                ](auto& shader, auto& vao) {
                    auto& scene = Registry::GetSingleton().getScene(HOST_INDEX_STACK);
                    auto& component = scene.getComponentByID(componentID);

                    auto& edgeColor = component.template getData<glm::vec4>("edgeColor");
                    auto& backgroundColor = component.template getData<glm::vec4>("backgroundColor");
                    auto& combinedThreshold = component.template getData<float>("combinedThreshold");
                    auto& lineThickness = component.template getData<float>("lineThickness");
                    auto& colorSensitivity = component.template getData<float>("internal_colorSensitivity");
                    auto& depthSensitivity = component.template getData<float>("internal_depthSensitivity");


                    auto& screenWidth = *shader.iRenderer->platformWindowPointer->renderWindowPointer->windowWidth;
                    auto& screenHeight = *shader.iRenderer->platformWindowPointer->renderWindowPointer->windowHeight;
                    glm::vec2 inverseScreenSize = (screenWidth > 0 && screenHeight > 0) ? glm::vec2(1.0f / screenWidth, 1.0f / screenHeight) : glm::vec2(0.0f);

                    // UBO structure remains the same
                    float edgeDetectionData[4 + 4 + 2 + 1 + 1 + 1 + 1] = {
                        edgeColor.r, edgeColor.g, edgeColor.b, edgeColor.a,
                        backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a,
                        inverseScreenSize.x, inverseScreenSize.y,
                        combinedThreshold,
                        lineThickness,
                        colorSensitivity,
                        depthSensitivity
                    };

                    shader.setBlock("EdgeDetectionParams", vao, edgeDetectionData);
                },
                .staticOnAttached = []() {
                    auto& sf = ShaderFactory::GetSingleton();
                    sf.addHook(
                        ShaderType::Fragment,
                        "layout",
                        "EdgeDetection",
                        [](Shader& shader, const auto& constants) -> std::string {
                            auto& sf = ShaderFactory::GetSingleton();
                            auto bindingIndex = sf.currentBindingIndex++;
                            auto uboSize = sizeof(float) * (4 + 4 + 2 + 1 + 1 + 1 + 1);
                            shader.addUBO(ShaderType::Fragment, "EdgeDetectionParams", bindingIndex, uboSize);

                            std::string uboString =
                                "layout(binding = " + std::to_string(bindingIndex) + ") uniform EdgeDetectionParams {\n"
                                "    vec4 edgeColor;\n"
                                "    vec4 backgroundColor;\n"
                                "    vec2 inverseScreenSize;\n"
                                "    float combinedThreshold;\n"
                                "    float lineThickness;\n"
                                "    float colorSensitivity;\n"
                                "    float depthSensitivity;\n"
                                "} edgeDetectionParams;\n";

                            // Add sampler declaration ONLY for DepthTexture as requested
                            bindingIndex = sf.currentBindingIndex++;
                            shader.addTexture(bindingIndex, ShaderType::Fragment, "DepthTexture");
                            uboString += "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler2D DepthTexture;\n";

                            // Assuming ColorTexture binding is handled elsewhere or implicitly before DepthTexture
                            // If ColorTexture needs explicit binding here, it should be added.
                            // For now, respecting the request to only show DepthTexture binding.

                            return uboString;
                        }
                    );

                    sf.addHook(
                        ShaderType::Fragment,
                        "postPostInMain",
                        "EdgeDetection",
                        [](auto& shader, const auto& constants) -> std::string {
                            // GLSL using Color Sobel + Depth Laplacian
                            return R"(
    // Calculate pixel offset based on line thickness and screen resolution
    vec2 texelSize = edgeDetectionParams.inverseScreenSize * edgeDetectionParams.lineThickness;
    vec2 offX = vec2(texelSize.x, 0.0);
    vec2 offY = vec2(0.0, texelSize.y);
    vec2 offXY = vec2(texelSize.x, texelSize.y);
    vec2 offYX = vec2(-texelSize.x, texelSize.y);

    // --- Color Edge Detection (Sobel - unchanged) ---
    // Assuming ColorTexture is available (e.g., bound at binding=0 implicitly or explicitly elsewhere)
    // If ColorTexture is not bound, this part will fail.
    float cSample0, cSample1, cSample2, cSample3, cSample4, cSample5, cSample6, cSample7;
    const vec3 lumaWeights = vec3(0.2126, 0.7152, 0.0722);
    // Using textureLod to sample mip level 0 explicitly, might help if mipmapping is enabled on ColorTexture
    cSample0 = dot(textureLod(ColorTexture, inUV - offXY, 0.0).rgb, lumaWeights);
    cSample1 = dot(textureLod(ColorTexture, inUV + offYX, 0.0).rgb, lumaWeights);
    cSample2 = dot(textureLod(ColorTexture, inUV - offYX, 0.0).rgb, lumaWeights);
    cSample3 = dot(textureLod(ColorTexture, inUV + offXY, 0.0).rgb, lumaWeights);
    cSample4 = dot(textureLod(ColorTexture, inUV - offY, 0.0).rgb, lumaWeights);
    cSample5 = dot(textureLod(ColorTexture, inUV + offY, 0.0).rgb, lumaWeights);
    cSample6 = dot(textureLod(ColorTexture, inUV - offX, 0.0).rgb, lumaWeights);
    cSample7 = dot(textureLod(ColorTexture, inUV + offX, 0.0).rgb, lumaWeights);
    float Gx_color = (cSample1 + 2.0 * cSample7 + cSample3) - (cSample0 + 2.0 * cSample6 + cSample2);
    float Gy_color = (cSample2 + 2.0 * cSample5 + cSample3) - (cSample0 + 2.0 * cSample4 + cSample1);
    float gradientMagnitudeColor = length(vec2(Gx_color, Gy_color));


    // --- Depth Edge Detection (Laplacian Filter) ---
    float centerDepth = textureLod(DepthTexture, inUV, 0.0).r;
    // Sample neighbours for Laplacian (4 neighbours version)
    float dN = textureLod(DepthTexture, inUV - offY, 0.0).r;
    float dE = textureLod(DepthTexture, inUV + offX, 0.0).r;
    float dS = textureLod(DepthTexture, inUV + offY, 0.0).r;
    float dW = textureLod(DepthTexture, inUV - offX, 0.0).r;

    /*// Calculate Laplacian (approximation using 4 neighbours)
    // L = (dN + dS + dE + dW) - 4 * centerDepth
    float laplacianDepth = (dN + dS + dE + dW) - 4.0 * centerDepth;
    // Use absolute value as we only care about the magnitude of the change
    float gradientMagnitudeDepth = abs(laplacianDepth);*/

    // Alternative: 8-neighbour Laplacian (more sensitive to diagonals)
    float dNE = textureLod(DepthTexture, inUV - offY + offX, 0.0).r;
    float dSE = textureLod(DepthTexture, inUV + offY + offX, 0.0).r;
    float dSW = textureLod(DepthTexture, inUV + offY - offX, 0.0).r;
    float dNW = textureLod(DepthTexture, inUV - offY - offX, 0.0).r;
    float laplacianDepth = (dN + dNE + dE + dSE + dS + dSW + dW + dNW) - 8.0 * centerDepth;
    float gradientMagnitudeDepth = abs(laplacianDepth);
    /**/


    // --- Normalize Gradients & Combine ---
    float normalizedColor = gradientMagnitudeColor / max(edgeDetectionParams.colorSensitivity, 0.001);
    float normalizedDepth = gradientMagnitudeDepth / max(edgeDetectionParams.depthSensitivity, 0.00001);

    // Take the strongest normalized signal
    float strongestNormalizedGradient = max(normalizedColor, normalizedDepth);


    // --- Antialiasing & Final Color ---
    float aaWidth = edgeDetectionParams.combinedThreshold * 0.5;
    float finalEdgeAlpha = smoothstep(edgeDetectionParams.combinedThreshold - aaWidth,
                                      edgeDetectionParams.combinedThreshold + aaWidth,
                                      strongestNormalizedGradient);

    vec4 baseColor;
    // Sample base color again (needed if not using background color)
    // Use textureLod again for consistency if ColorTexture might have mipmaps
    if (edgeDetectionParams.backgroundColor.a < 0.01) {
        baseColor = textureLod(ColorTexture, inUV, 0.0);
    } else {
        baseColor = edgeDetectionParams.backgroundColor;
    }

    FragColor = mix(baseColor, edgeDetectionParams.edgeColor, finalEdgeAlpha);

    if (edgeDetectionParams.backgroundColor.a < 0.01) {
         FragColor.a = mix(baseColor.a, edgeDetectionParams.edgeColor.a, finalEdgeAlpha);
    } else {
         FragColor.a = mix(edgeDetectionParams.backgroundColor.a, edgeDetectionParams.edgeColor.a, finalEdgeAlpha);
    }

)";
                        }
                    );
                } // end staticOnAttached
            }; // end edgeDetectionStageCreateInfo

            scene.postProcessingPipeline.addStage(40.f, edgeDetectionStageCreateInfo);

        } // end onAttachedFunction
    }; // end SceneComponentCreateInfo

    return info;
}
