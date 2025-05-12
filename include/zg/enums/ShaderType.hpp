#pragma once
#include <unordered_map>
#include <string>
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
    inline static std::unordered_map<ShaderType, std::string> shaderTypeStringMap = {
        {ShaderType::Unknown, "Unknown"},
        {ShaderType::Vertex, "Vertex"},
        {ShaderType::Geometry, "Geometry"},
        {ShaderType::Fragment, "Fragment"},
        {ShaderType::TessellationControl, "TessellationControl"},
        {ShaderType::TessellationEvaluation, "TessellationEvaluation"},
        {ShaderType::Compute, "Compute"}
    };
}