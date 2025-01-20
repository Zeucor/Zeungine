#pragma once
#include "./Shader.hpp"
#include <memory>
#include "../GLWindow.hpp"
namespace anex::modules::gl::shaders
{
	struct ShaderManager
  {
    static Shader& getShaderByID(GLWindow &window, uint32_t id);
    static std::pair<uint32_t, std::shared_ptr<Shader>> getShaderByConstants(GLWindow &window, const RuntimeConstants &constants, const std::vector<Shader::ShaderType> &shaderTypes = {Shader::ShaderType::Vertex, Shader::ShaderType::Fragment});
  };
}