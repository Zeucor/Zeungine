#pragma once
#include <unordered_map>
#include "Shader.hpp"
#include <zg/Singleton.hpp>
namespace zg::shaders
{
    void register_zg_shader_hooks();
	struct ShaderFactory : Singleton<ShaderFactory>
	{
		using RuntimeHooksMap = std::unordered_map<std::string, std::map<uint32_t, Shader::ShaderHook>>;
		using ShaderHookMap = std::map<std::string, RuntimeHooksMap>;
		using ShaderHooksMap = std::unordered_map<ShaderType, ShaderHookMap>;
		using ShaderHookInfo = std::tuple<ShaderType, std::string, std::string>;
		using ShaderHookInfoMap = std::unordered_map<uint32_t, ShaderHookInfo>;
		using ShaderTypeMap = std::unordered_map<ShaderType, uint32_t>;
		using ShaderNameMap = std::unordered_map<ShaderType, std::string>;
		ShaderHooksMap hooks;
		uint32_t hooksCount;
		ShaderHookInfoMap shaderHookInfos;
		static ShaderTypeMap shaderTypes;
		static ShaderNameMap shaderNames;
		uint32_t currentInLayoutIndex;
		uint32_t currentOutLayoutIndex;
		uint32_t currentBindingIndex;
		ShaderMap generateShaderMap(const RuntimeConstants &constants, Shader &shader,
										   const std::vector<ShaderType> &shaderTypes);
		ShaderPair generateShader(const ShaderType &shaderType, const RuntimeConstants &runtimeConstants,
										 Shader &shader);
		void appendHooks(std::string &shaderString, RuntimeHooksMap &runtimeHooks, const RuntimeConstants &constants,
								Shader &shader);
		bool compileShader(Shader &shader, ShaderType shaderType, ShaderPair &shaderPair);
		bool compileProgram(Shader &shader);
		void deleteProgram(Shader &shader);
		uint32_t addHook(const ShaderType &shaderType, const std::string& hookName,
								const std::string& runtimeConstant, const Shader::ShaderHook &hook);
		void deleteHook(uint32_t id);
	};

}; // namespace zg::shaders