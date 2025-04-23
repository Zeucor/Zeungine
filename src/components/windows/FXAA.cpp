#include <zg/components/windows/FXAA.hpp>
#include <zg/Registry.hpp>
#include <zg/shaders/ShaderFactory.hpp>
using namespace zg::components::windows;
//
// TODO: statically register hooks
//
// Component definition
zg::components::windows::WindowComponentCreateInfo zg::components::windows::FXAAFactory(float edgeThresholdMin, float edgeThreshold, int edgeSearchSteps, float subpixQuality)
{
    WindowComponentCreateInfo info{
        .name = "FXAA",
        .onAttachedFunction = [](auto& component)
        {
            auto& window = Registry::getWindow(component.hostIndexStack);
            zg::PostProcessingStageCreateInfo fxaaStageCreateInfo{
                .name = "FXAA",
                .inputs = {"ColorTexture"},
                .outputs = {{"ColorTexture", textures::Framebuffer::AttachmentType::Color}}
            };
            window.postProcessingPipeline.addStage(100.f, fxaaStageCreateInfo);
        }
    };
    return info;
}