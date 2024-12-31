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
		std::unordered_map<std::string, std::tuple<uint32_t, GLuint>> ssboBindings;
    ShaderMap shaders;
    GLuint program = 0;
		Shader(const RuntimeConstants& constants);
		~Shader();
		void use(const bool &useProgram);
		void addSSBO(const std::string& name, const uint32_t &bindingIndex);
		void addUBO(const std::string &name, const uint32_t &bindingIndex);
		template <typename T>
		void setUniform(const std::string &name, const T& value, const uint32_t &size = 0)
		{
			auto pointerSize = size ? size : sizeof(value);
			GLint location = glGetUniformLocation(program, name.c_str());
			GLcheck("glGetUniformLocation");
			auto pointer = &value;
			if (location == -1)
			{
				return;
			}
			if constexpr (std::is_same_v<T, bool>)
			{
				glUniform1i(location, (int32_t)(*(bool*)pointer));
				GLcheck("glUniform1i");
				return;
			}
			else if constexpr (std::is_same_v<T, int32_t>)
			{
				glUniform1i(location, *(int32_t*)pointer);
				GLcheck("glUniform1i");
				return;
			}
			else if constexpr (std::is_same_v<T, uint32_t>)
			{
				glUniform1ui(location, *(uint32_t*)pointer);
				GLcheck("glUniform1ui");
				return;
			}
			else if constexpr (std::is_same_v<T, float*>)
			{
				glUniform1fv(location, pointerSize, &((float**)pointer)[0][0]);
				GLcheck("glUniform1fv");
				return;
			}
			else if constexpr (std::is_same_v<T, float>)
			{
				glUniform1f(location, *(float*)pointer);
				GLcheck("glUniform1f");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::vec2>)
			{
				glUniform2fv(location, 1, &(*(glm::vec2*)pointer)[0]);
				GLcheck("glUniform2fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::vec3>)
			{
				glUniform3fv(location, 1, &(*(glm::vec3*)pointer)[0]);
				GLcheck("glUniform3fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::vec4>)
			{
				glUniform4fv(location, 1, &(*(glm::vec4*)pointer)[0]);
				GLcheck("glUniform4fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::mat2>)
			{
				glUniformMatrix2fv(location, 1, GL_FALSE, &(*(glm::mat2*)pointer)[0][0]);
				GLcheck("glUniformMatrix2fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::mat3>)
			{
				glUniformMatrix3fv(location, 1, GL_FALSE, &(*(glm::mat3*)pointer)[0][0]);
				GLcheck("glUniformMatrix3fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::mat4>)
			{
				glUniformMatrix4fv(location, 1, GL_FALSE, &(*(glm::mat4*)pointer)[0][0]);
				GLcheck("glUniformMatrix4fv");
				return;
			}
			throw std::runtime_error("setUniform: unsupported type");
		};
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
			GLcheck("glUniformBlockBinding");
			auto& uboBufferIndex = std::get<1>(uboBinding);
			glBindBuffer(GL_UNIFORM_BUFFER, uboBufferIndex);
			GLcheck("glBindBuffer");
			auto pointerSize = size ? size : sizeof(value);
			glBufferData(GL_UNIFORM_BUFFER, pointerSize, &value, GL_STATIC_DRAW);
			GLcheck("glBufferData");
			glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, uboBufferIndex, 0, pointerSize);
			GLcheck("glBindBufferRange");
		};
		void setSSBO(const std::string &name, const void *pointer, const uint32_t &size);
  };
}