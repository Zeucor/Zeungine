#pragma once
#include "SceneComponent.hpp"
namespace zg::components::scenes
{
    SceneComponentCreateInfo SMAAFactory(float threshold = 0.1f, float maxSearchSteps = 16, float maxSearchStepsDiag = 8, float cornerRounding = 25);
}