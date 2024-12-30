#pragma once
#include "./Shader.hpp"
#include <memory>
namespace anex::modules::gl::shaders
{
	struct ShaderManager
  {
    static std::unordered_map<uint32_t, std::shared_ptr<Shader>> shaders;
		static std::unordered_map<std::size_t, std::pair<uint32_t, std::shared_ptr<Shader>>> shadersByHash;
    static uint32_t shaderCount;
    static Shader& getShaderByID(const uint32_t &id);
    static std::pair<uint32_t, std::shared_ptr<Shader>> getShaderByConstants(const RuntimeConstants &constants);
    static void deleteShaders();
  };
}