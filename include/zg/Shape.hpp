#pragma once
#include <cstdint>
#include <zg/shaders/Shader.hpp>
#include <zg/Singleton.hpp>
namespace zg
{
    enum class ShapeType
    {
        NoShape = 0,
        Box = 1,
        SDF = 2,
        Mesh = 3,
        PlaneXZ = 4,
        PlaneXY = 5,
        PlaneYZ = 6,
    };
    inline static std::unordered_map<ShapeType, uint32_t> shapeVerticeCounts = {
        { ShapeType::Box, 36 },
        { ShapeType::PlaneXZ, 6 },
        { ShapeType::PlaneXY, 6 },
        { ShapeType::PlaneYZ, 6 },
        { ShapeType::SDF, 6 }
    };
    struct GLEntity
    {
        int32_t shape_type;
        int32_t material_index;
        int32_t vertex_offset;
        int32_t padding = 0;
        // int32_t normal_offset;
        int32_t uv2_offset;
        int32_t uv3_offset;
        int32_t meta_int;
        float meta_float;
        glm::vec4 meta_vec4;
    };
}