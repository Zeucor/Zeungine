#pragma once
#include <cstdint>
namespace zg
{
    enum class ShapeType
    {
        Box = 1,
        Plane,
        Mesh
    };
    struct GLEntity
    {
        uint32_t shape_type;
        uint32_t material_index;
        uint32_t vertex_offset;
    };
}