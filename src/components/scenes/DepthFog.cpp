
#include <zg/components/scenes/DepthFog.hpp>
#include <zg/Window.hpp>
#include <zg/Registry.hpp>
#include <zg/shaders/Shader.hpp>
#include <zg/shaders/ShaderFactory.hpp>
using namespace zg::components::scenes;
using zg::Registry;
using zg::shaders::Shader;
using zg::shaders::ShaderFactory;
using zg::shaders::ShaderType;
using zg::PostProcessingStageCreateInfo;
SceneComponentCreateInfo zg::components::scenes::DepthFogFactory() {
    SceneComponentCreateInfo info{
        .name = "DepthFog",
        .onAttachedFunction = [](auto& component) {
            component.template make<glm::vec4>("fogColor", 0.5f, 0.6f, 0.7f, 1.0f); // Default cool grey fog
            component.template make<float>("fogStartDistance", 15.0f); // For Linear fog
            component.template make<float>("fogEndDistance", 80.0f); // For Linear fog
            component.template make<float>("fogDensity", 0.07f);      // For Exponential fog types
            component.template make<FogType>("fogType", FogType::Linear); //FogType::Exponential); // Default fog type
            component.template make<bool>("IgnoreMaxDepth", true);
            PostProcessingStageCreateInfo depthFogStageCreateInfo{
                .name = "DepthFog",
                .inputs = {"ColorTexture", "DepthTexture"},
                .outputs = {{"ColorTexture", zg::textures::Framebuffer::AttachmentType::Color, textures::Texture::AddressMode::ClampToEdge}},
                .constants = {"DepthFog", "LinearizeDepthUtil"},
                .setShaderConstants = [
                    HOST_INDEX_STACK = component.HOST_INDEX_STACK,
                    componentID = component.ID
                ](auto& shader, auto& vao) {
                    auto& scene = Registry::getScene(HOST_INDEX_STACK);
                    auto& component = scene.getComponentByID(componentID);
                    auto& fogColor = component.template getData<glm::vec4>("fogColor");
                    auto& fogStartDistance = component.template getData<float>("fogStartDistance");
                    auto& fogEndDistance = component.template getData<float>("fogEndDistance");
                    auto& fogDensity = component.template getData<float>("fogDensity");
                    auto& fogType = component.template getData<FogType>("fogType");
                    auto& ignoreMaxDepth = component.template getData<bool>("IgnoreMaxDepth");
                    auto& nearPlane = scene.projectionPointer->nearPlane;
                    auto& farPlane = scene.projectionPointer->farPlane;
                    float fogData[11] = {
                        fogColor.r,
                        fogColor.g,
                        fogColor.b,
                        fogColor.a,
                        fogStartDistance,
                        fogEndDistance,
                        fogDensity,
                        static_cast<float>(static_cast<int>(fogType)),
                        nearPlane,
                        farPlane,
                        static_cast<float>(static_cast<int>(ignoreMaxDepth))
                    };
                    shader.setBlock("DepthFogParams", vao, fogData);
                },
                .staticOnAttached = []() {
                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "layout",
                        "DepthFog",
                        [](Shader& shader, const auto& constants) -> std::string {
                            uint32_t bindingIndex = ShaderFactory::currentBindingIndex++;
                            size_t uboSize = sizeof(float) * 11;
                            shader.addUBO(ShaderType::Fragment, "DepthFogParams", bindingIndex, uboSize);
                            std::string uboString =
                                "layout(binding = " + std::to_string(bindingIndex) + ") uniform DepthFogParams {\n"
                                "    vec4 fogColor;\n"
                                "    float fogStartDistance;\n"
                                "    float fogEndDistance;\n"
                                "    float fogDensity;\n"
                                "    float fogType;\n"
                                "    float nearPlane;\n"
                                "    float farPlane;\n"
                                "    float ignoreMaxDepth;\n"
                                "} depthFogParams;\n";
                            uint32_t depthTexBinding = ShaderFactory::currentBindingIndex++;
                            shader.addTexture(depthTexBinding, ShaderType::Fragment, "DepthTexture");
                            uboString += "layout(binding = " + std::to_string(depthTexBinding) + ") uniform sampler2D DepthTexture;\n";
                            return uboString;
                        }
                    );

                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "preMain",
                        "LinearizeDepthUtil",
                        [](Shader& shader, const auto& constants) -> std::string {
                            std::string string;
                            string += R"(
float linearizeDepth(float depthSample, float near, float far) {)";
                            if (shader.iRenderer->renderer == zg::RENDERER_VULKAN)
                            {
                                string += R"(
    float z_ndc = depthSample;
                                )";
                            }
                            else
                            {
                                string += R"(
    float z_ndc = depthSample * 2.0 - 1.0;
                                )";
                            }
                            string += R"(
    float z_eye = (2.0 * near * far) / (far + near - z_ndc * (far - near));
    return abs(z_eye);
})";
                            return string;
                        }
                    );

                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "postPostInMain",
                        "DepthFog",
                        [](auto& shader, const auto& constants) -> std::string {
                            return R"(
    vec4 originalColor = texture(ColorTexture, inUV);
    float depthSample = texture(DepthTexture, inUV).r;
    // Check for background/skybox (depth == 1.0) - skip fog for sky
    if (depthFogParams.ignoreMaxDepth == 1.0 && depthSample >= 1.0) {
        FragColor = originalColor; // No fog on the far plane/sky
        return; // Early exit using return (or skip the mix)
    }
    // Linearize the depth sample to get view-space distance
    float linearDistance = linearizeDepth(depthSample, depthFogParams.nearPlane, depthFogParams.farPlane);
    // Calculate fog factor [0, 1] (0 = no fog, 1 = full fog)
    float fogFactor = 0.0;
    const int FOG_LINEAR = 0;
    const int FOG_EXP = 1;
    const int FOG_EXP2 = 2;
    if (depthFogParams.fogType == FOG_LINEAR) {
        // Linear fog: (distance - start) / (end - start)
        fogFactor = (linearDistance - depthFogParams.fogStartDistance) / (depthFogParams.fogEndDistance - depthFogParams.fogStartDistance);
    } else if (depthFogParams.fogType == FOG_EXP) {
        // Exponential fog: 1.0 - exp(-(distance * density))
        // Ensure density is positive
        fogFactor = 1.0 - exp(-linearDistance * abs(depthFogParams.fogDensity));
    } else if (depthFogParams.fogType == FOG_EXP2) {
        // Exponential squared fog: 1.0 - exp(- (distance * density)^2 )
        float term = linearDistance * abs(depthFogParams.fogDensity);
        fogFactor = 1.0 - exp(-(term * term));
    }
    // Clamp fog factor to [0, 1] range
    fogFactor = clamp(fogFactor, 0.0, 1.0);
    // Blend the original color with the fog color
    FragColor = mix(originalColor, depthFogParams.fogColor, fogFactor);
    // Preserve original alpha if needed, or use fog alpha
    FragColor.a = originalColor.a; // Usually keep original alpha)";
                        }
                    );
                } // end staticOnAttached
            }; // end depthFogStageCreateInfo

            // Add the stage to the window's post-processing pipeline
            // Place it after most effects, but potentially before UI/final tonemapping
            auto& scene = Registry::getScene(component.HOST_INDEX_STACK);
            scene.postProcessingPipeline.addStage(70.f, depthFogStageCreateInfo);

        } // end onAttachedFunction
    }; // end WindowComponentCreateInfo

    return info;
}