#pragma once
#include "./Shader.hpp"
#include <memory>
#include "../Window.hpp"
namespace zg::shaders
{
  struct ShaderManager
  {
    static Shader &getShaderByID(Window &window, uint32_t id);
    static std::pair<uint32_t, std::shared_ptr<Shader>> getShaderByConstants(Window &window, const RuntimeConstants &constants, const std::vector<ShaderType> &shaderTypes = {ShaderType::Vertex, ShaderType::Fragment});
  };
}