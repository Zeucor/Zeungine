#include <zg/Registry.hpp>
#include <zg/components/windows/SMAA.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/Window.hpp>
using namespace zg::components::windows;
zg::components::windows::WindowComponentCreateInfo zg::components::windows::SMAAFactory(
    float threshold,
    float maxSearchSteps,
    float maxSearchStepsDiag,
    float cornerRounding
)
{
	WindowComponentCreateInfo info{
        .name = "SMAA",
        .onAttachedFunction = [
            threshold,
            maxSearchSteps,
            maxSearchStepsDiag,
            cornerRounding
        ](auto& component)
        {
            auto& window = zg::Registry::getWindow(component.hostIndexStack);
            zg::PostProcessingStageCreateInfo fxaaStageCreateInfo{
                .name = "SMAA",
                .inputs = {"ColorTexture"},
                .outputs = {{"ColorTexture", zg::textures::Framebuffer::AttachmentType::Color}},
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
                                "} fxaaValues;\n";
                            bindingIndex = shaders::ShaderFactory::currentBindingIndex++;
                            shader.addUBO(shaders::ShaderType::Fragment, "InverseScreenSize", bindingIndex, sizeof(glm::vec2));
                            string +=
                                "layout(binding = " + std::to_string(bindingIndex) + ") uniform InverseScreenSize {\n" +
                                "  vec2 size;\n" +
                                "} inverseScreenSize;\n";
                            return string;
                        }
                    );
                    shaders::ShaderFactory::addHook(
                        shaders::ShaderType::Fragment,
                        "postPostInMain",
                        "SMAA",
                        [](auto& shader, const auto& constants) -> std::string {
                            return "";
                        }
                    );
                    shaders::ShaderFactory::addHook(
                        shaders::ShaderType::Fragment,
                        "postMain",
                        "SMAA",
                        [](auto& shader, const auto& constants) -> std::string {
                            return "";
                        }
                    );
                }
            };
            window.postProcessingPipeline.addStage(100.f, fxaaStageCreateInfo);
        }
    };
	return info;
}
