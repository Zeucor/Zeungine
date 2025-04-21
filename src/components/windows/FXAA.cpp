#include <zg/components/windows/FXAA.hpp>
#include <zg/Registry.hpp>
using namespace zg::components::windows;
zg::components::windows::WindowComponentCreateInfo zg::components::windows::FXAAFactory(float edgeThresholdMin, float edgeThreshold, int edgeSearchSteps, float subpixQuality)
{
    WindowComponentCreateInfo info{
        .name = "FXAA",
        .onAttachedFunction = [](auto& component)
        {
            auto& window = Registry::getWindow(component.hostIndexStack);
            zg::PostProcessingStageCreateInfo fxaaStageCreateInfo{
                .name = "FXAA",
                .inputs = {"Color"},
                .outputs = {"Color"}
            };
            window.postProcessingPipeline.addStage(100.f, fxaaStageCreateInfo);
        }
    };
    return info;
}