#include "SceneComponent.hpp"
namespace zg::components::scenes {
    enum class FogType : int {
        Linear = 0,
        Exponential = 1,
        ExponentialSquared = 2
    };
    SceneComponentCreateInfo DepthFogFactory();
}