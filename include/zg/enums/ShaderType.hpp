#pragma once
namespace zg::shaders
{
    enum class ShaderType
    {
        Unknown = 0,
        Vertex,
        Geometry,
        Fragment,
        TessellationControl,
        TessellationEvaluation,
        Compute
    };
}