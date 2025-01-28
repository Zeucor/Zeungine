#pragma once
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <zg/Window.hpp>
#include <zg/glm.hpp>
#include "../common.hpp"
#include "RuntimeConstants.hpp"
#include <zg/renderers/GLRenderer.hpp>
namespace zg::textures
{
	struct Texture;
}

namespace zg::shaders
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
		Window &window;
		ShaderMap shaders;
		GLuint program = 0;
		Shader(Window &window, const RuntimeConstants& constants, const std::vector<ShaderType> &shaderTypes = {ShaderType::Vertex, ShaderType::Fragment});
		~Shader();
		void bind() const;
		void unbind() const;
		void addSSBO(const std::string_view name, uint32_t bindingIndex);
		void addUBO(const std::string_view name, uint32_t bindingIndex);

		template <typename T>
		void setUniform(const std::string_view name, const T& value, uint32_t size = 0)
		{
			auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iVendorRenderer);
			auto pointerSize = size ? size : sizeof(value);
			GLint location = glRenderer.glContext->GetUniformLocation(program, name.data());
			GLcheck(glRenderer, "glGetUniformLocation");
			auto pointer = &value;
			if (location == -1)
			{
				return;
			}
			if constexpr (std::is_same_v<T, bool>)
			{
				glRenderer.glContext->Uniform1i(location, (int32_t)(*(bool*)pointer));
				GLcheck(glRenderer, "glUniform1i");
				return;
			}
			else if constexpr (std::is_same_v<T, int32_t>)
			{
				glRenderer.glContext->Uniform1i(location, *(int32_t*)pointer);
				GLcheck(glRenderer, "glUniform1i");
				return;
			}
			else if constexpr (std::is_same_v<T, uint32_t>)
			{
				glRenderer.glContext->Uniform1ui(location, *(uint32_t*)pointer);
				GLcheck(glRenderer, "glUniform1ui");
				return;
			}
			else if constexpr (std::is_same_v<T, float*>)
			{
				glRenderer.glContext->Uniform1fv(location, pointerSize, &((float**)pointer)[0][0]);
				GLcheck(glRenderer, "glUniform1fv");
				return;
			}
			else if constexpr (std::is_same_v<T, float>)
			{
				glRenderer.glContext->Uniform1f(location, *(float*)pointer);
				GLcheck(glRenderer, "glUniform1f");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::vec2>)
			{
				glRenderer.glContext->Uniform2fv(location, 1, &(*(glm::vec2*)pointer)[0]);
				GLcheck(glRenderer, "glUniform2fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::vec3>)
			{
				glRenderer.glContext->Uniform3fv(location, 1, &(*(glm::vec3*)pointer)[0]);
				GLcheck(glRenderer, "glUniform3fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::vec4>)
			{
				glRenderer.glContext->Uniform4fv(location, 1, &(*(glm::vec4*)pointer)[0]);
				GLcheck(glRenderer, "glUniform4fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::mat2>)
			{
				glRenderer.glContext->UniformMatrix2fv(location, 1, GL_FALSE, &(*(glm::mat2*)pointer)[0][0]);
				GLcheck(glRenderer, "glUniformMatrix2fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::mat3>)
			{
				glRenderer.glContext->UniformMatrix3fv(location, 1, GL_FALSE, &(*(glm::mat3*)pointer)[0][0]);
				GLcheck(glRenderer, "glUniformMatrix3fv");
				return;
			}
			else if constexpr (std::is_same_v<T, glm::mat4>)
			{
				glRenderer.glContext->UniformMatrix4fv(location, 1, GL_FALSE, &(*(glm::mat4*)pointer)[0][0]);
				GLcheck(glRenderer, "glUniformMatrix4fv");
				return;
			}
			throw std::runtime_error("setUniform: unsupported type");
		};

		template <typename T>
		void setBlock(const std::string_view name, const T& value, uint32_t size = 0)
		{
			auto &glRenderer = *std::dynamic_pointer_cast<GLRenderer>(window.iVendorRenderer);
			auto blockIndex = glRenderer.glContext->GetUniformBlockIndex(program, name.data());
			if (blockIndex == -1)
			{
				return;
			}
			auto& uboBinding = uboBindings[name];
			auto& bindingIndex = std::get<0>(uboBinding);
			glRenderer.glContext->UniformBlockBinding(program, blockIndex, bindingIndex);
			GLcheck(glRenderer, "glUniformBlockBinding");
			auto& uboBufferIndex = std::get<1>(uboBinding);
			glRenderer.glContext->BindBuffer(GL_UNIFORM_BUFFER, uboBufferIndex);
			GLcheck(glRenderer, "glBindBuffer");
			auto pointerSize = size ? size : sizeof(value);
			glRenderer.glContext->BufferData(GL_UNIFORM_BUFFER, pointerSize, &value, GL_STATIC_DRAW);
			GLcheck(glRenderer, "glBufferData");
			glRenderer.glContext->BindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, uboBufferIndex, 0, pointerSize);
			GLcheck(glRenderer, "glBindBufferRange");
		};
		void setSSBO(const std::string_view name, const void* pointer, uint32_t size);
		void setTexture(const std::string_view name, const textures::Texture& texture, const int32_t& unit);
	};
}
