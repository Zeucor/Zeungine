#pragma once
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <zg/Window.hpp>
#include <zg/enums/ShaderType.hpp>
#include <zg/glm.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include "../common.hpp"
#include "RuntimeConstants.hpp"
#include <zg/vaos/VAO.hpp>
namespace zg::textures
{
	struct Texture;
}
namespace zg::shaders
{
	struct Shader
	{
		using ShaderHook = std::function<std::string(Shader &, const RuntimeConstants &)>;
		Window &window;
		RuntimeConstants constants;
		size_t hash = 0;
		void *rendererData = 0;
		bool compiled = false;
		Shader(Window &window,
			   const RuntimeConstants &constants,
			   const std::vector<ShaderType> &shaderTypes = {ShaderType::Vertex, ShaderType::Fragment});
		~Shader();
		void bind(Entity &entity);
		void unbind();
		void addSSBO(ShaderType shaderType, const std::string_view name, uint32_t bindingIndex);
		void addUBO(ShaderType shaderType, const std::string_view name, uint32_t bindingIndex, uint32_t bufferSize, uint32_t descriptorCount = 1, bool isArray = false);
		void addTexture(uint32_t bindingIndex, shaders::ShaderType shaderType, std::string_view textureName, uint32_t descriptorCount = 1);
		template <typename T>
		void setUniform(const std::string_view name, vaos::VAO &vao, const T &value, uint32_t size = 0)
		{
			auto pointerSize = size ? size : sizeof(value);
			enums::EUniformType uniformType;
			if constexpr (std::is_same_v<T, bool>)
			{
				uniformType = enums::EUniformType::_1b;
			}
			else if constexpr (std::is_same_v<T, int32_t>)
			{
				uniformType = enums::EUniformType::_1i;
			}
			else if constexpr (std::is_same_v<T, uint32_t>)
			{
				uniformType = enums::EUniformType::_1ui;
			}
			else if constexpr (std::is_same_v<T, float *>)
			{
				uniformType = enums::EUniformType::_1fv;
			}
			else if constexpr (std::is_same_v<T, float>)
			{
				uniformType = enums::EUniformType::_1f;
			}
			else if constexpr (std::is_same_v<T, glm::vec2>)
			{
				uniformType = enums::EUniformType::_2fv;
			}
			else if constexpr (std::is_same_v<T, glm::vec3>)
			{
				uniformType = enums::EUniformType::_3fv;
			}
			else if constexpr (std::is_same_v<T, glm::vec4>)
			{
				uniformType = enums::EUniformType::_4fv;
			}
			else if constexpr (std::is_same_v<T, glm::mat2>)
			{
				uniformType = enums::EUniformType::_Matrix2fv;
			}
			else if constexpr (std::is_same_v<T, glm::mat3>)
			{
				uniformType = enums::EUniformType::_Matrix3fv;
			}
			else if constexpr (std::is_same_v<T, glm::mat4>)
			{
				uniformType = enums::EUniformType::_Matrix4fv;
			}
			else
			{
				throw std::runtime_error("setUniform: unsupported type");
			}
			window.iRenderer->setUniform(*this, vao, name, &value, pointerSize, uniformType);
		};

		template <typename T>
		void setBlock(const std::string_view name, vaos::VAO &vao, const T &value, uint32_t size = 0)
		{
			auto pointerSize = size ? size : sizeof(value);
			window.iRenderer->setBlock(*this, vao, name, &value, pointerSize);
		};
		void setSSBO(const std::string_view name, vaos::VAO &vao, const void *pointer, uint32_t size);
		void setTexture(const std::string_view name, vaos::VAO &vao, const textures::Texture &texture, const int32_t unit);
	};
#if defined(USE_GL) || defined(USE_EGL)
	struct GLShaderImpl
	{
		std::unordered_map<std::string_view, std::tuple<uint32_t, uint32_t>> uboBindings;
		std::unordered_map<std::string_view, std::tuple<uint32_t, uint32_t>> ssboBindings;
		ShaderMap shaders;
		GLuint program = 0;
	};
#endif
} // namespace zg::shaders