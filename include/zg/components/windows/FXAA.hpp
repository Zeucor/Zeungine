#pragma once
#include "WindowComponent.hpp"
namespace zg::components::windows
{
    WindowComponentCreateInfo FXAAFactory(float edgeThresholdMin = 0.0312f, float edgeThreshold = 0.125f, float edgeSearchSteps = 12, float subpixQuality = 0.75f);
}