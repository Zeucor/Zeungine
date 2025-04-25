#include <zg/Registry.hpp>
#include <zg/components/windows/Bloom.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/Window.hpp>
using namespace zg::components::windows;
using zg::shaders::Shader;
using zg::shaders::ShaderType;
using zg::shaders::ShaderFactory;
zg::components::windows::WindowComponentCreateInfo zg::components::windows::BloomFactory()
{
	WindowComponentCreateInfo info{
        .name = "Bloom",
        .onAttachedFunction = [](auto& component)
        {
            auto& window = zg::Registry::getWindow(component.hostIndexStack);
            auto& bloomColorMultiplier = component.template make<float>("bloomColorMultiplier", 1.00f);
            auto& bloomCoefficients = component.template make<glm::vec3>("bloomCoefficients", glm::vec3(0.2126, 0.7152, 0.0722));
            auto& bloomThreshold = component.template make<float>("bloomThreshold", 0.7f);
            auto& intensity = component.template make<float>("intensity", 0.8f);
            auto& Pi2 = component.template make<float>("Pi2", 6.28318530718f);
            auto& Directions = component.template make<float>("Directions", 16.0f);
            auto& Quality = component.template make<float>("Quality", 5.0f);
            auto& Size = component.template make<float>("Size", 88.0f);
            zg::PostProcessingStageCreateInfo bloomBrightExtractStageCreateInfo{
                .name = "BloomBrightExtract",
                .inputs = {"ColorTexture"},
                .outputs = {{"BrightTexture", zg::textures::Framebuffer::AttachmentType::Color}},
                .constants = {"BloomBrightExtract"},
                .setShaderConstants = [
                    hostIndexStack = component.hostIndexStack,
                    componentID = component.ID
                ](Shader& shader, auto& vao)
                {
                    auto& window = Registry::getWindow(hostIndexStack);
                    auto& component = window.getComponentByID(componentID);
                    auto& bloomColorMultiplier = component.template getData<float>("bloomColorMultiplier");
                    auto& bloomCoefficients = component.template getData<glm::vec3>("bloomCoefficients");
                    auto& bloomThreshold = component.template getData<float>("bloomThreshold");
                    float brightExtract[8] = {
                        bloomColorMultiplier,
                        0,
                        0,
                        0,
                        bloomCoefficients.r,
                        bloomCoefficients.g,
                        bloomCoefficients.b,
                        bloomThreshold
                    };
                    shader.setBlock("BrightExtract", vao, brightExtract);
                },
                .staticOnAttached = []()
                {
                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "layout",
                        "BloomBrightExtract",
                        [](Shader& shader, const auto& constants) -> std::string {
                            
                            uint32_t bindingIndex = shaders::ShaderFactory::currentBindingIndex++;
                            shader.addUBO(shaders::ShaderType::Fragment, "BrightExtract", bindingIndex, sizeof(float)*8);
                            std::string string;
                            string += "layout(binding = " + std::to_string(bindingIndex) + ") uniform BrightExtract {\n" +
                                "  float bloomColorMultiplier;\n"
                                "  vec3 bloomCoefficients;\n"
                                "  float bloomThreshold;\n"
                                "} brightExtract;\n";
                            return string;
                        }
                    );
                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "postPostInMain",
                        "BloomBrightExtract",
                        [](auto& shader, const auto& constants) -> std::string {
                            return R"(  float brightness = dot(vec3(FragColor) * brightExtract.bloomColorMultiplier, brightExtract.bloomCoefficients);
    if (brightness < brightExtract.bloomThreshold) {
        FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    })";
                        }
                    );
                }
            };
            zg::PostProcessingStageCreateInfo bloomBlurStageCreateInfo{
                .name = "BloomBlur",
                .inputs = {"BrightTexture"},
                .outputs = {{"BlurTexture", zg::textures::Framebuffer::AttachmentType::Color}},
                .constants = {"BloomBlur"},
                .setShaderConstants = [
                    hostIndexStack = component.hostIndexStack,
                    componentID = component.ID
                ](Shader& shader, auto& vao)
                {
                    auto& window = Registry::getWindow(hostIndexStack);
                    auto& component = window.getComponentByID(componentID);
                    auto& Pi2 = component.template getData<float>("Pi2");
                    auto& Directions = component.template getData<float>("Directions");
                    auto& Quality = component.template getData<float>("Quality");
                    auto& Size = component.template getData<float>("Size");
                    float blur[6] = {
                        shader.iRenderer->platformWindowPointer->renderWindowPointer->windowWidth,
                        shader.iRenderer->platformWindowPointer->renderWindowPointer->windowHeight,
                        Pi2,
                        Directions,
                        Quality,
                        Size
                    };
                    shader.setBlock("Blur", vao, blur);
                },
                .staticOnAttached = []()
                {
                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "layout",
                        "BloomBlur",
                        [](Shader& shader, const auto& constants) -> std::string {
                            uint32_t bindingIndex = shaders::ShaderFactory::currentBindingIndex++;
                            shader.addUBO(shaders::ShaderType::Fragment, "Blur", bindingIndex, sizeof(float) * 6);
                            std::string string;
                            string +=
                                "layout(binding = " + std::to_string(bindingIndex) + ") uniform Blur {\n" +
                                "  vec2 screenSize;\n"
                                "  float Pi2;\n"
                                "  float Directions;\n"
                                "  float Quality;\n"
                                "  float Size;\n"
                                "} blur;\n";
                            string += R"(
)";
                            return string;
                        }
                    );
                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "layout",
                        "BrightTexture",
                        [](Shader& shader, const auto& constants) -> std::string {
                            auto bindingIndex = ShaderFactory::currentBindingIndex++;
                            shader.addTexture(bindingIndex, ShaderType::Fragment, "BrightTexture");
                            return "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler2D BrightTexture;";
                        }
                    );
                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "layout",
                        "BloomBlur",
                        [](Shader& shader, const auto& constants) -> std::string {
                            return "layout(location = " + std::to_string(ShaderFactory::currentOutLayoutIndex++) +
                            ") out vec4 BlurColor;\n";
                        }
                    );
                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "postPostInMain",
                        "BloomBlur",
                        [](auto& shader, const auto& constants) -> std::string {
                            return R"(
    vec2 Radius = blur.Size / blur.screenSize;
    vec4 bloomColor = texture(BrightTexture, inUV);
    for (float d = 0.0; d < blur.Pi2; d += blur.Pi2 / blur.Directions)
        for (float i = 1.0 / blur.Quality; i <= 1.0; i += 1.0 / blur.Quality)
            bloomColor += texture(BrightTexture, inUV + vec2(cos(d), sin(d)) * Radius * i);
    bloomColor /= blur.Quality * blur.Directions - 15.0;
    BlurColor = bloomColor;)";
                        }
                    );
                }
            };
            zg::PostProcessingStageCreateInfo bloomCombineStageCreateInfo{
                .name = "BloomCombine",
                .inputs = {"ColorTexture", "BlurTexture"},
                .outputs = {{"ColorTexture", zg::textures::Framebuffer::AttachmentType::Color}},
                .constants = {"BloomCombine"},
                .setShaderConstants = [
                    hostIndexStack = component.hostIndexStack,
                    componentID = component.ID
                ](Shader& shader, auto& vao)
                {
                    auto& window = Registry::getWindow(hostIndexStack);
                    auto& component = window.getComponentByID(componentID);
                    auto& intensity = component.template getData<float>("intensity");
                    float bloomCombine[1] = {
            intensity
                    };
                    shader.setBlock("BloomCombine", vao, bloomCombine);
                },
                .staticOnAttached = []()
                {
                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "layout",
                        "BlurTexture",
                        [](Shader& shader, const auto& constants) -> std::string {
                            auto bindingIndex = ShaderFactory::currentBindingIndex++;
                            shader.addTexture(bindingIndex, ShaderType::Fragment, "BlurTexture");
                            return "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler2D BlurTexture;";
                        }
                    );
                    
                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "layout",
                        "BloomCombine",
                        [](Shader& shader, const auto& constants) -> std::string {
                            uint32_t bindingIndex = shaders::ShaderFactory::currentBindingIndex++;
                            shader.addUBO(shaders::ShaderType::Fragment, "BloomCombine", bindingIndex, sizeof(float)*1);
                            return "layout(binding = " + std::to_string(bindingIndex) + ") uniform BloomCombine {\n" +
                                "  float intensity;\n"
                                "} bloomCombine;\n";
                        }
                    );
                    ShaderFactory::addHook(
                        ShaderType::Fragment,
                        "postPostInMain",
                        "BloomCombine",
                        [](auto& shader, const auto& constants) -> std::string {
                            return R"(  vec4 Color = texture(ColorTexture, inUV);
    vec4 bloomColor = texture(BlurTexture, inUV);
    FragColor = Color + (bloomColor * bloomCombine.intensity);)";
                        }
                    );
                }
            };
            window.postProcessingPipeline.addStage(50.f, bloomBrightExtractStageCreateInfo);
            window.postProcessingPipeline.addStage(51.f, bloomBlurStageCreateInfo);
            window.postProcessingPipeline.addStage(52.f, bloomCombineStageCreateInfo);
        }
    };
	return info;
}
