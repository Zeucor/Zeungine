#pragma once
#include <zg/Scene.hpp>
namespace zg::components::scenes
{
    enum KeyScheme
    {
        UDLRSH = 1,
        WSADSC = 2
    };
    SceneComponentCreateInfo ViewQuadKeyControlFactory(KeyScheme keyScheme, float force);
}