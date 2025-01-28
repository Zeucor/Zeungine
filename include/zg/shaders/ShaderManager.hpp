#pragma once
#include "./Shader.hpp"
#include <memory>
#include "../Window.hpp"
namespace zg::shaders
{
	struct ShaderManager
  {
    static Shader& getShaderByID(Window &window, uint32_t id);
    static std::pair<uint32_t, std::shared_ptr<Shader>> getShaderByConstants(Window &window, const RuntimeConstants &constants, const std::vector<Shader::ShaderType> &shaderTypes = {Shader::ShaderType::Vertex, Shader::ShaderType::Fragment});
  };
}