#pragma once
#include <unordered_map>
#include "Shader.hpp"
namespace zg::shaders
{
	struct ShaderFactory
	{
		using RuntimeHooksMap = std::unordered_map<std::string_view, std::map<uint32_t, Shader::ShaderHook>>;
		using ShaderHookMap = std::map<std::string_view, RuntimeHooksMap>;
		using ShaderHooksMap = std::unordered_map<ShaderType, ShaderHookMap>;
		using ShaderHookInfo = std::tuple<ShaderType, std::string_view, std::string_view>;
		using ShaderHookInfoMap = std::unordered_map<uint32_t, ShaderHookInfo>;
		using ShaderTypeMap = std::unordered_map<ShaderType, uint32_t>;
		using ShaderNameMap = std::unordered_map<ShaderType, std::string_view>;
		static ShaderHooksMap hooks;
		static uint32_t hooksCount;
		static ShaderHookInfoMap shaderHookInfos;
		static ShaderTypeMap shaderTypes;
		static ShaderNameMap shaderNames;
		static uint32_t currentInLayoutIndex;
		static uint32_t currentOutLayoutIndex;
		static uint32_t currentBindingIndex;
		static ShaderMap generateShaderMap(const RuntimeConstants &constants, Shader &shader,
										   const std::vector<ShaderType> &shaderTypes);
		static ShaderPair generateShader(const ShaderType &shaderType, const RuntimeConstants &runtimeConstants,
										 Shader &shader);
		static void appendHooks(std::string &shaderString, RuntimeHooksMap &runtimeHooks, const RuntimeConstants &constants,
								Shader &shader);
		static bool compileShader(Shader &shader, ShaderType shaderType, ShaderPair &shaderPair);
		static bool compileProgram(Shader &shader);
		static void deleteProgram(Shader &shader);
		static uint32_t addHook(const ShaderType &shaderType, const std::string_view hookName,
								const std::string_view runtimeConstant, const Shader::ShaderHook &hook);
		static void deleteHook(uint32_t id);
	};
}; // namespace zg::shaders