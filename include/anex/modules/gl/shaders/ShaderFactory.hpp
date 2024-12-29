#pragma once
#include "Shader.hpp"
#include <unordered_map>
namespace anex::modules::gl::shaders
{
	struct ShaderFactory
	{
    using RuntimeHooksMap = std::unordered_map<std::string, std::map<uint32_t, Shader::ShaderHook>>;
  	using ShaderHookMap = std::map<std::string, RuntimeHooksMap>;
    using ShaderHooksMap = std::unordered_map<Shader::ShaderType, ShaderHookMap>;
		using ShaderHookInfo = std::tuple<Shader::ShaderType, std::string, std::string>;
		using ShaderHookInfoMap = std::unordered_map<uint32_t, ShaderHookInfo>;
    using ShaderTypeMap = std::unordered_map<Shader::ShaderType, GLuint>;
    using ShaderNameMap = std::unordered_map<Shader::ShaderType, std::string>;
    static ShaderHooksMap hooks;
		static uint32_t hooksCount;
		static ShaderHookInfoMap shaderHookInfos;
    static ShaderTypeMap shaderTypes;
    static ShaderNameMap shaderNames;
    static uint32_t currentInLayoutIndex;
    static uint32_t currentOutLayoutIndex;
    static Shader::ShaderMap generateShaderMap(const RuntimeConstants &constants);
		static Shader::ShaderPair generateShader(const Shader::ShaderType &shaderType, const RuntimeConstants &runtimeConstants);
    static void appendHooks(std::string &shaderString, RuntimeHooksMap &runtimeHooks, const RuntimeConstants &constants);
    static bool compileShader(const Shader::ShaderType &shaderType, Shader::ShaderPair &shaderPair);
    static bool compileProgram(const Shader::ShaderMap &shaderMap, GLuint &program);
		static const bool checkCompileErrors(const GLuint& id, const bool& isShader, const char* shaderType);
    static void deleteProgram(Shader &shader);
    static uint32_t addHook(const Shader::ShaderType &shaderType, const std::string &hookName, const std::string &runtimeConstant, const Shader::ShaderHook &hook);
		static void deleteHook(const uint32_t &id);
	};
};