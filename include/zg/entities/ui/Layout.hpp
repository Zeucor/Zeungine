#pragma once
#include <zg/Entity.hpp>
namespace zg::entities::ui
{
    enum class LayoutDimension
    {
        Horizontal = 1,
        Vertical = 2,
        Depth = 4
    };
    EntityCreateInfo LayoutFactory(const std::string& name, LayoutDimension layoutDimension, glm::vec3 position, glm::vec3 size, IRenderer* irenderer, bool isNDCSize = true);
}