#pragma once
#include "WindowComponent.hpp"
namespace zg::components::windows
{
    WindowComponentCreateInfo SMAAFactory(float threshold = 0.1f, float maxSearchSteps = 16, float maxSearchStepsDiag = 8, float cornerRounding = 25);
}