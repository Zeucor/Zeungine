#pragma once
#include "SceneComponent.hpp"
namespace zg::components::scenes
{
    SceneComponentCreateInfo FXAAFactory(float edgeThresholdMin = 0.0312f, float edgeThreshold = 0.125f, float edgeSearchSteps = 12, float subpixQuality = 0.75f);
}