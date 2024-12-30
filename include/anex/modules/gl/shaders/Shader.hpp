#pragma once
#include "../common.hpp"
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include "../../../glm.hpp"
namespace anex::modules::gl::shaders
{
	using RuntimeConstants = std::vector<std::string>;
	struct Shader
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
    using ShaderPair = std::pair<std::string, GLuint>;
    using ShaderMap = std::map<ShaderType, ShaderPair>;
    using ShaderHook = std::function<std::string(Shader&, const RuntimeConstants&)>;
		std::unordered_map<std::string, std::tuple<uint32_t, GLuint>> uboBindings;
    ShaderMap shaders;
    GLuint program = 0;
		Shader(const RuntimeConstants& constants);
		~Shader();
		void use(const bool &useProgram);
		template <typename T>
		void setBlock(const std::string &name, const T& value, const uint32_t &size = 0)
		{
			auto blockIndex = glGetUniformBlockIndex(program, name.c_str());
			if (blockIndex == -1)
			{
				return;
			}
			auto& uboBinding = uboBindings[name];
			auto& bindingIndex = std::get<0>(uboBinding);
			glUniformBlockBinding(program, blockIndex, bindingIndex);
			auto& uboBufferIndex = std::get<1>(uboBinding);
			glBindBuffer(GL_UNIFORM_BUFFER, uboBufferIndex);
			auto pointerSize = size ? size : sizeof(value);
			glBufferData(GL_UNIFORM_BUFFER, pointerSize, &value, GL_STATIC_DRAW);
			glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, uboBufferIndex, 0, pointerSize);
		};
		void addUBO(const std::string &name, const uint32_t &size, const uint32_t &bindingIndex);
  };
}