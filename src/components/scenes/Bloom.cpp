#include <zg/Registry.hpp>
#include <zg/components/scenes/Bloom.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/Window.hpp>
using namespace zg;
using namespace zg::components::scenes;
using zg::shaders::Shader;
using zg::shaders::ShaderType;
using zg::shaders::ShaderFactory;
SceneComponentCreateInfo zg::components::scenes::BloomFactory()
{
	SceneComponentCreateInfo info{
        .name = "Bloom",
        .onAttachedFunction = [](auto& component)
        {
            auto& scene = Registry::getScene(component.HOST_INDEX_STACK);
            auto& bloomColorMultiplier = component.template make<float>("bloomColorMultiplier", 1.00f);
            auto& bloomCoefficients = component.template make<glm::vec3>("bloomCoefficients", glm::vec3(0.2126, 0.7152, 0.0722));
            auto& bloomThreshold = component.template make<float>("bloomThreshold", 0.7f);
            auto& intensity = component.template make<float>("intensity", 0.8f);
            auto& Pi2 = component.template make<float>("Pi2", 6.28318530718f);
            auto& Directions = component.template make<float>("Directions", 16.0f);
            auto& Quality = component.template make<float>("Quality", 5.0f);
            auto& Size = component.template make<float>("Size", 88.0f);
            PostProcessingStageCreateInfo bloomBrightExtractStageCreateInfo{
                .name = "BloomBrightExtract",
                .inputs = {"ColorTexture"},
                .outputs = {{"BrightTexture", textures::Framebuffer::AttachmentType::Color, textures::Texture::AddressMode::ClampToEdge}},
                .constants = {"BloomBrightExtract"},
                .setShaderConstants = [
                    HOST_INDEX_STACK = component.HOST_INDEX_STACK,
                    componentID = component.ID
                ](Shader& shader, auto& vao)
                {
                    auto& scene = Registry::getScene(HOST_INDEX_STACK);
                    auto& component = scene.getComponentByID(componentID);
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
                    auto& sf = ShaderFactory::GetSingleton();
                    sf.addHook(
                        ShaderType::Fragment,
                        "layout",
                        "BloomBrightExtract",
                        [](Shader& shader, const auto& constants) -> std::string {
                            
                            auto& sf = ShaderFactory::GetSingleton();
                            uint32_t bindingIndex = sf.currentBindingIndex++;
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
                    sf.addHook(
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
            PostProcessingStageCreateInfo bloomBlurStageCreateInfo{
                .name = "BloomBlur",
                .inputs = {"BrightTexture"},
                .outputs = {{"BlurTexture", textures::Framebuffer::AttachmentType::Color, textures::Texture::AddressMode::ClampToEdge}},
                .constants = {"BloomBlur"},
                .setShaderConstants = [
                    HOST_INDEX_STACK = component.HOST_INDEX_STACK,
                    componentID = component.ID
                ](Shader& shader, auto& vao)
                {
                    auto& scene = Registry::getScene(HOST_INDEX_STACK);
                    auto& component = scene.getComponentByID(componentID);
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
                    auto& sf = ShaderFactory::GetSingleton();
                    sf.addHook(
                        ShaderType::Fragment,
                        "layout",
                        "BloomBlur",
                        [](Shader& shader, const auto& constants) -> std::string {
                            auto& sf = ShaderFactory::GetSingleton();
                            uint32_t bindingIndex = sf.currentBindingIndex++;
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
                    sf.addHook(
                        ShaderType::Fragment,
                        "layout",
                        "BrightTexture",
                        [](Shader& shader, const auto& constants) -> std::string {
                            auto& sf = ShaderFactory::GetSingleton();
                            auto bindingIndex = sf.currentBindingIndex++;
                            shader.addTexture(bindingIndex, ShaderType::Fragment, "BrightTexture");
                            return "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler2D BrightTexture;";
                        }
                    );
                    sf.addHook(
                        ShaderType::Fragment,
                        "layout",
                        "BloomBlur",
                        [](Shader& shader, const auto& constants) -> std::string {
                            auto& sf = ShaderFactory::GetSingleton();
                            return "layout(location = " + std::to_string(sf.currentOutLayoutIndex++) +
                            ") out vec4 BlurColor;\n";
                        }
                    );
                    sf.addHook(
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
            PostProcessingStageCreateInfo bloomCombineStageCreateInfo{
                .name = "BloomCombine",
                .inputs = {"ColorTexture", "BlurTexture"},
                .outputs = {{"ColorTexture", textures::Framebuffer::AttachmentType::Color, textures::Texture::AddressMode::ClampToEdge}},
                .constants = {"BloomCombine"},
                .setShaderConstants = [
                    HOST_INDEX_STACK = component.HOST_INDEX_STACK,
                    componentID = component.ID
                ](Shader& shader, auto& vao)
                {
                    auto& scene = Registry::getScene(HOST_INDEX_STACK);
                    auto& component = scene.getComponentByID(componentID);
                    auto& intensity = component.template getData<float>("intensity");
                    float bloomCombine[1] = {
            intensity
                    };
                    shader.setBlock("BloomCombine", vao, bloomCombine);
                },
                .staticOnAttached = []()
                {
                    auto& sf = ShaderFactory::GetSingleton();
                    sf.addHook(
                        ShaderType::Fragment,
                        "layout",
                        "BlurTexture",
                        [](Shader& shader, const auto& constants) -> std::string {
                            auto& sf = ShaderFactory::GetSingleton();
                            auto bindingIndex = sf.currentBindingIndex++;
                            shader.addTexture(bindingIndex, ShaderType::Fragment, "BlurTexture");
                            return "layout(binding = " + std::to_string(bindingIndex) + ") uniform sampler2D BlurTexture;";
                        }
                    );
                    
                    sf.addHook(
                        ShaderType::Fragment,
                        "layout",
                        "BloomCombine",
                        [](Shader& shader, const auto& constants) -> std::string {
                            auto& sf = ShaderFactory::GetSingleton();
                            uint32_t bindingIndex = sf.currentBindingIndex++;
                            shader.addUBO(shaders::ShaderType::Fragment, "BloomCombine", bindingIndex, sizeof(float)*1);
                            return "layout(binding = " + std::to_string(bindingIndex) + ") uniform BloomCombine {\n" +
                                "  float intensity;\n"
                                "} bloomCombine;\n";
                        }
                    );
                    sf.addHook(
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
            scene.postProcessingPipeline.addStage(50.f, bloomBrightExtractStageCreateInfo);
            scene.postProcessingPipeline.addStage(51.f, bloomBlurStageCreateInfo);
            scene.postProcessingPipeline.addStage(52.f, bloomCombineStageCreateInfo);
        }
    };
	return info;
}
