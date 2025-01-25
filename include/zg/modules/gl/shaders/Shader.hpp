#pragma once
#include "../common.hpp"
#include <map>
#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include "../../../glm.hpp"
#include <zg/modules/gl/GLWindow.hpp>
#include "RuntimeConstants.hpp"
namespace zg::modules::gl::textures
{
	struct Texture;
}

namespace zg::modules::gl::shaders
{
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
		std::unordered_map<std::string_view, std::tuple<uint32_t, GLuint>> uboBindings;
		std::unordered_map<std::string_view, std::tuple<uint32_t, GLuint>> ssboBindings;
		GLWindow &window;
		ShaderMap shaders;
		GLuint program = 0;
		Shader(GLWindow &window, const RuntimeConstants& constants, const std::vector<ShaderType> &shaderTypes = {ShaderType::Vertex, ShaderType::Fragment});
		~Shader();
		void bind() const;
		void unbind() const;
		void addSSBO(const std::string_view name, uint32_t bindingIndex);
		void addUBO(const std::string_view name, uint32_t bindingIndex);

		template <typename T>
		void setUniform(const std::string_view name, const T& value, uint32_t size = 0)
		{
			auto pointerSize = size ? size : sizeof(value);
			GLint location = window.glContext->GetUniformLocation(program, name.data());
			GLcheck(window, "glGetUniformLocation");
			auto pointer = &value;
			if (location == -1)
			{
				return;
			}
			if constexpr (std::is_same_v<T, bool>)
			{
				window.glContext->Uniform1i(location, (int32_t)(*(bool*)pointer));
				GLcheck(window, "glUniform1i");
				return;
			}
			else if constexpr (std::is_same_v<T, int32_t>)
			{
				window.glContext->Uniform1i(location, *(int32_t*)pointer);
				GLcheck(window, "glUniform1i");
				return;
			}
			else if constexpr (std::is_same_v<T, uint32_t>)
			{
				window.glContext->Uniform1ui(location, *(uint32_t*)pointer);
				GLcheck(window, "glUniform1ui");
				return;
			}
			else if constexpr (std::is_same_v<T, float*>)
			{
				window.glContext->Uniform1fv(location, pointerSize, &((float**)pointer)[0][0]);
				GLcheck(window, "glUniform1fv");
				return;
			}
			else if constexpr (std::is_same_v<T, float>)
			{
				window.glContext->Uniform1f(location, *(float*)pointer);
				GLcheck(window, "glUniform1f");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::vec2>)
			{
				window.glContext->Uniform2fv(location, 1, &(*(glm::vec2*)pointer)[0]);
				GLcheck(window, "glUniform2fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::vec3>)
			{
				window.glContext->Uniform3fv(location, 1, &(*(glm::vec3*)pointer)[0]);
				GLcheck(window, "glUniform3fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::vec4>)
			{
				window.glContext->Uniform4fv(location, 1, &(*(glm::vec4*)pointer)[0]);
				GLcheck(window, "glUniform4fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::mat2>)
			{
				window.glContext->UniformMatrix2fv(location, 1, GL_FALSE, &(*(glm::mat2*)pointer)[0][0]);
				GLcheck(window, "glUniformMatrix2fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::mat3>)
			{
				window.glContext->UniformMatrix3fv(location, 1, GL_FALSE, &(*(glm::mat3*)pointer)[0][0]);
				GLcheck(window, "glUniformMatrix3fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::mat4>)
			{
				window.glContext->UniformMatrix4fv(location, 1, GL_FALSE, &(*(glm::mat4*)pointer)[0][0]);
				GLcheck(window, "glUniformMatrix4fv");
				return;
			}
			throw std::runtime_error("setUniform: unsupported type");
		};

		template <typename T>
		void setBlock(const std::string_view name, const T& value, uint32_t size = 0)
		{
			auto blockIndex = window.glContext->GetUniformBlockIndex(program, name.data());
			if (blockIndex == -1)
			{
				return;
			}
			auto& uboBinding = uboBindings[name];
			auto& bindingIndex = std::get<0>(uboBinding);
			window.glContext->UniformBlockBinding(program, blockIndex, bindingIndex);
			GLcheck(window, "glUniformBlockBinding");
			auto& uboBufferIndex = std::get<1>(uboBinding);
			window.glContext->BindBuffer(GL_UNIFORM_BUFFER, uboBufferIndex);
			GLcheck(window, "glBindBuffer");
			auto pointerSize = size ? size : sizeof(value);
			window.glContext->BufferData(GL_UNIFORM_BUFFER, pointerSize, &value, GL_STATIC_DRAW);
			GLcheck(window, "glBufferData");
			window.glContext->BindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, uboBufferIndex, 0, pointerSize);
			GLcheck(window, "glBindBufferRange");
		};
		void setSSBO(const std::string_view name, const void* pointer, uint32_t size);
		void setTexture(const std::string_view name, const textures::Texture& texture, const int32_t& unit);
	};
}
