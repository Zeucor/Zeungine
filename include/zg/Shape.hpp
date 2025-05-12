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
        int32_t shape_type;
        int32_t material_index;
        int32_t vertex_offset;
        int32_t uv2_offset;
        int32_t uv3_offset;
    };
}