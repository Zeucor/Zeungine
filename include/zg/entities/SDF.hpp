#pragma once
#include <zg/Entity.hpp>
namespace zg
{
    void register_zg_sdfs();
    using sdf_function = std::function<float(const Entity&, glm::vec3)>;
    struct SDFRegistry : Singleton<SDFRegistry>
    {
    private:
        size_t total_sdf_count = 0;
        std::unordered_map<std::string, std::tuple<size_t, shaders::Shader::ShaderHook, std::string>> functionHooks;
        std::unordered_map<int32_t, sdf_function> cFunctions;
    public:
        int32_t register_sdf(const std::string& key, const shaders::Shader::ShaderHook& functionHook, const std::string& param_append_string = "");
        int32_t get_sdf_type(const std::string& key);
        void register_c_sdf(int32_t id, const sdf_function& sdf);
        sdf_function& get_sdf_function(int32_t id);
        size_t size();
        auto begin() { return functionHooks.begin(); }
        const auto begin() const { return functionHooks.begin(); }
        auto end() { return functionHooks.end(); }
        const auto end() const { return functionHooks.end(); }
    };
}
namespace zg::entities
{
    EntityCreateInfo SDFFactory(const std::string& sdf_key, glm::vec4 color, const std::string& name = "SDF", glm::vec3 position = {0, 0, 0}, glm::quat rotation = {1, 0, 0, 0}, glm::vec3 scale = {1, 1, 1},
        const shaders::RuntimeConstants constants = {});
}