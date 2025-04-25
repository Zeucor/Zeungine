#include <zg/Registry.hpp>
#include <zg/components/windows/EdgeDetection.hpp>
#include <zg/shaders/Shader.hpp>
#include <zg/shaders/ShaderFactory.hpp>
using namespace zg::components::windows;
using zg::Registry;
using zg::shaders::Shader;
using zg::shaders::ShaderFactory;
using zg::shaders::ShaderType;
WindowComponentCreateInfo zg::components::windows::EdgeDetectionFactory() {
    WindowComponentCreateInfo info{
        .name = "EdgeDetection",
        .onAttachedFunction = [](auto& component) {
            auto& window = zg::Registry::getWindow(component.hostIndexStack);
            component.template make<glm::vec4>("edgeColor", 0.0f, 0.0f, 0.0f, 1.0f); // Black edges
            component.template make<glm::vec4>("backgroundColor", 1.0f, 1.0f, 1.0f, 0.0f); // Transparent background (overlay)
            component.template make<float>("threshold", 0.05f); // Edge sensitivity

            zg::PostProcessingStageCreateInfo edgeDetectionStageCreateInfo{
                .name = "EdgeDetection",
                .inputs = {"ColorTexture"},
                .outputs = {{"ColorTexture", zg::textures::Framebuffer::AttachmentType::Color}},
                .constants = {"EdgeDetection"},
                .setShaderConstants = [
                    hostIndexStack = component.hostIndexStack,
                    componentID = component.ID
                ](auto& shader, auto& vao) {
                    auto& window = Registry::getWindow(hostIndexStack);
                    auto& component = window.getComponentByID(componentID);

                    auto& edgeColor = component.template getData<glm::vec4>("edgeColor");
                    auto& backgroundColor = component.template getData<glm::vec4>("backgroundColor");
                    auto& threshold = component.template getData<float>("threshold");

                    auto screenWidth = shader.iRenderer->platformWindowPointer->renderWindowPointer->windowWidth;
                    auto screenHeight = shader.iRenderer->platformWindowPointer->renderWindowPointer->windowHeight;
                    glm::vec2 inverseScreenSize = (screenWidth > 0 && screenHeight > 0) ? glm::vec2(1.0f / screenWidth, 1.0f / screenHeight) : glm::vec2(0.0f);

                    float edgeDetectionData[4+4+2+1] = {
                        edgeColor.r,
                        edgeColor.g,
                        edgeColor.b,
                        edgeColor.a,
    
                        backgroundColor.r,
                        backgroundColor.g,
                        backgroundColor.b,
                        backgroundColor.a,
    
                        inverseScreenSize.x,
                        inverseScreenSize.y,
    
                        threshold
                    };

                    shader.setBlock("EdgeDetectionParams", vao, edgeDetectionData);
                },
                .staticOnAttached = []() {
                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "layout",
                        "EdgeDetection",
                        [](Shader& shader, const auto& constants) -> std::string {
                            auto bindingIndex = ShaderFactory::currentBindingIndex++;
                            auto uboSize = sizeof(float) * (4+4+2+1);
                            shader.addUBO(ShaderType::Fragment, "EdgeDetectionParams", bindingIndex, uboSize);

                            std::string uboString =
                                "layout(binding = " + std::to_string(bindingIndex) + ") uniform EdgeDetectionParams {\n"
                                "    vec4 edgeColor;\n"
                                "    vec4 backgroundColor;\n"
                                "    vec2 inverseScreenSize;\n"
                                "    float threshold;\n"
                                "} edgeDetectionParams;\n";

                            return uboString;
                        }
                    );

                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "postPostInMain",
                        "EdgeDetection",
                        [](auto& shader, const auto& constants) -> std::string {
                            return R"(
    vec2 off1 = vec2(1.0, 0.0) * edgeDetectionParams.inverseScreenSize;
    vec2 off2 = vec2(0.0, 1.0) * edgeDetectionParams.inverseScreenSize;
    float sample0, sample1, sample2, sample3, sample4, sample5, sample6, sample7, sample8;
    // Calculate luminance for color-based edge detection
    // Using standard luminance weights (similar to bloom)
    const vec3 lumaWeights = vec3(0.2126, 0.7152, 0.0722);
    sample0 = dot(texture(ColorTexture, inUV - off1 - off2).rgb, lumaWeights);
    sample1 = dot(texture(ColorTexture, inUV + off1 - off2).rgb, lumaWeights);
    sample2 = dot(texture(ColorTexture, inUV - off1 + off2).rgb, lumaWeights);
    sample3 = dot(texture(ColorTexture, inUV + off1 + off2).rgb, lumaWeights);
    sample4 = dot(texture(ColorTexture, inUV - off2).rgb, lumaWeights);
    sample5 = dot(texture(ColorTexture, inUV + off2).rgb, lumaWeights);
    sample6 = dot(texture(ColorTexture, inUV - off1).rgb, lumaWeights);
    sample7 = dot(texture(ColorTexture, inUV + off1).rgb, lumaWeights);
    // Sobel filter kernels for X and Y gradients
    float Gx = (sample1 + 2.0 * sample7 + sample3) - (sample0 + 2.0 * sample6 + sample2);
    float Gy = (sample2 + 2.0 * sample5 + sample3) - (sample0 + 2.0 * sample4 + sample1);
    // Calculate gradient magnitude
    float gradientMagnitude = sqrt(Gx * Gx + Gy * Gy);
    if (gradientMagnitude > edgeDetectionParams.threshold) {
        FragColor = edgeDetectionParams.edgeColor; // Output edge color
    } else {
        // If background is transparent, keep original color, otherwise use background color
        if (edgeDetectionParams.backgroundColor.a < 0.01) {
             // Keep original color (read it if not already in FragColor)
             // FragColor = texture(ColorTexture, inUV); // Assuming FragColor wasn't modified yet
        } else {
             FragColor = edgeDetectionParams.backgroundColor; // Output background color
        }
        // If you want to overlay edges ON the original color:
        // vec4 originalColor = texture(ColorTexture, inUV); // Sample original
        // FragColor = mix(originalColor, edgeDetectionParams.edgeColor, step(edgeDetectionParams.threshold, gradientMagnitude));
    })";
                        }
                    );
                } // end staticOnAttached
            }; // end edgeDetectionStageCreateInfo

            window.postProcessingPipeline.addStage(40.f, edgeDetectionStageCreateInfo);

        } // end onAttachedFunction
    }; // end WindowComponentCreateInfo

    return info;
}