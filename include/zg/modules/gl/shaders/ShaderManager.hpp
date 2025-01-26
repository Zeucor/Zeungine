#pragma once
#include "./Shader.hpp"
#include <memory>
#include "../RenderWindow.hpp"
namespace zg::modules::gl::shaders
{
	struct ShaderManager
  {
    static Shader& getShaderByID(RenderWindow &window, uint32_t id);
    static std::pair<uint32_t, std::shared_ptr<Shader>> getShaderByConstants(RenderWindow &window, const RuntimeConstants &constants, const std::vector<Shader::ShaderType> &shaderTypes = {Shader::ShaderType::Vertex, Shader::ShaderType::Fragment});
  };
}