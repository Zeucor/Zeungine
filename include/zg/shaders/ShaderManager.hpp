#pragma once
#include "./Shader.hpp"
#include <memory>
#include "../Window.hpp"
namespace zg::shaders
{
  struct ShaderManager
  {
    static Shader &getShaderByID(IRenderer* iRenderer, uint32_t id);
    static std::pair<uint32_t, std::shared_ptr<Shader>> getShaderByConstants(IRenderer* iRenderer,
                                                                             const RuntimeConstants &constants,
                                                                             void *data,
                                                                             const std::vector<ShaderType> &shaderTypes = {ShaderType::Vertex, ShaderType::Fragment});
  };
}