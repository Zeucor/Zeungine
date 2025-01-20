#pragma once
#include "Shader.hpp"
#include <unordered_map>
namespace anex::modules::gl::shaders
{
	struct ShaderFactory
	{
    using RuntimeHooksMap = std::unordered_map<std::string_view, std::map<uint32_t, Shader::ShaderHook>>;
  	using ShaderHookMap = std::map<std::string_view, RuntimeHooksMap>;
    using ShaderHooksMap = std::unordered_map<Shader::ShaderType, ShaderHookMap>;
		using ShaderHookInfo = std::tuple<Shader::ShaderType, std::string_view, std::string_view>;
		using ShaderHookInfoMap = std::unordered_map<uint32_t, ShaderHookInfo>;
    using ShaderTypeMap = std::unordered_map<Shader::ShaderType, GLuint>;
    using ShaderNameMap = std::unordered_map<Shader::ShaderType, std::string_view>;
    static ShaderHooksMap hooks;
		static uint32_t hooksCount;
		static ShaderHookInfoMap shaderHookInfos;
    static ShaderTypeMap shaderTypes;
    static ShaderNameMap shaderNames;
    static uint32_t currentInLayoutIndex;
    static uint32_t currentOutLayoutIndex;
		static uint32_t currentBindingIndex;
    static Shader::ShaderMap generateShaderMap(const RuntimeConstants &constants, Shader &shader, const std::vector<Shader::ShaderType> &shaderTypes);
		static Shader::ShaderPair generateShader(const Shader::ShaderType &shaderType, const RuntimeConstants &runtimeConstants, Shader &shader);
    static void appendHooks(std::string& shaderString, RuntimeHooksMap &runtimeHooks, const RuntimeConstants &constants, Shader &shader);
    static bool compileShader(Shader &shader, const Shader::ShaderType &shaderType, Shader::ShaderPair &shaderPair);
    static bool compileProgram(Shader &shader, const Shader::ShaderMap &shaderMap, GLuint &program);
		static const bool checkCompileErrors(Shader &shader, const GLuint& id, bool isShader, const char* shaderType);
    static void deleteProgram(Shader &shader);
    static uint32_t addHook(const Shader::ShaderType &shaderType, const std::string_view hookName, const std::string_view runtimeConstant, const Shader::ShaderHook &hook);
		static void deleteHook(uint32_t id);
	};
};