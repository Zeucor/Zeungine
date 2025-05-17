#pragma once
#include <zg/DrawList.hpp>
#include <zg/KeyIDVector.hpp>
#include <unordered_map>
#include <zg/glm.hpp>
#include <string>
#include <vector>
#include <unordered_set>
#include "sorted.hpp"
namespace zg
{
    namespace shaders
    {
        struct Shader;
    }
    struct Scene;
    template<typename KeyT, typename ValueT, typename HostT>
    struct transform_registry
    {
        using getter_function = std::function<ValueT(HostT&)>;
        inline static std::unordered_map<KeyT, getter_function> keyValueUsingHostMap = {};
        static ValueT get_value(const KeyT& key, HostT& host)
        {
            auto iter = keyValueUsingHostMap.find(key);
            if (iter == keyValueUsingHostMap.end())
            {
                return ValueT(0.0);
            }
            return iter->second(host);
        }
        static bool set_getter_function(const KeyT& key, const getter_function& getter)
        {
            keyValueUsingHostMap[key] = getter;
            return true;
        }
        static std::vector<std::string> extract_transform_keys(const std::vector<std::string>& keys)
        {
            std::vector<std::string> ex_tk;
            for (const auto& key : keys)
            {
                auto iter = keyValueUsingHostMap.find(key);
                if (iter != keyValueUsingHostMap.end())
                {
                    ex_tk.push_back(key);
                }
            }
            return ex_tk;
        }
        static bool initialized;
    };
    using entity_mat4_transform_registry = transform_registry<std::string, glm::mat4, Entity>;
#define SHADER_BATCH_MAIN 0
#define SHADER_BATCH_DIRECTIONAL_LIGHT 1
#define SHADER_BATCH_POINT_LIGHT 2
#define SHADER_BATCH_SPOT_LIGHT 3
    struct InstancedDraw
    {
    private:
        struct InstanceBatch
        {
            size_t m_meshID;
            using transform_keys = std::vector<std::string>;
            transform_keys m_transform_keys;
            std::unordered_set<size_t> entities;
            std::unordered_map<std::string, std::vector<glm::mat4>> mvp_transforms;
            glm::vec3 cameraPosition;
            InstanceBatch(size_t meshID);
            void addEntity(const size_t entity_ID);
            void removeEntity(const size_t entity_ID);
            void draw(Scene& scene, shaders::Shader* shaderPointer = 0);
            void update_transforms();
        };
        /**
         * @brief opaqueBatches Key = MeshID, value = InstanceBatch (entity transform vectors)
         */
        std::unordered_map<size_t, InstanceBatch> opaqueBatches;
        /**
         * @brief transparentBatches Key = MeshID, value = InstanceBatch (entity transform vectors)
         */
        std::unordered_map<size_t, InstanceBatch> transparentBatches;
        /**
         * @brief shaderBatches Key = shaderPointer, value = unordered_set<MeshID>
         */
        std::unordered_map<size_t, std::unordered_map<shaders::Shader*, std::unordered_set<size_t>>> bID_shaderBatches;
    public:
        void draw(
            Scene& scene,
            const OpaqueDrawList& opaqueDrawList,
            const TransparentDrawList& transparentDrawList,
            size_t& oldOpaqueHash,
            size_t& oldTransparentHash,
            shaders::Shader* shaderPointer = 0
        );
        void drawMulti(
            size_t batchID,
            Scene& scene,
            const OpaqueDrawList& opaqueDrawList,
            const TransparentDrawList& transparentDrawList,
            size_t& oldOpaqueHash,
            size_t& oldTransparentHash,
            shaders::Shader* shaderPointer = 0,
            const std::unordered_map<std::string, glm::mat4>& transformOverrides = {},
            const std::unordered_map<std::string, std::function<void(Scene&, shaders::Shader&, Mesh&)>>& shaderSets = {}
        );
        void addEntity(const Entity& entity);
        void removeEntity(const Entity& entity);
        void removeMesh(size_t meshID);
    };
}