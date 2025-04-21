#pragma once
#include <string>
#include <vector>
#include <map>
#include <zg/textures/Texture.hpp>
#include <zg/shaders/RuntimeConstants.hpp>
#include <zg/KeyIDVector.hpp>
namespace zg
{
    struct TextureOutputRegistry
    {
        static std::unordered_map<std::string, std::shared_ptr<textures::Texture>> map;
        void registerOutput(const std::string& key, std::shared_ptr<textures::Texture>& texture);
        void deregisterOutput(const std::string& key);
    };
    struct PostProcessingStageCreateInfo
    {
        std::string name;
        std::vector<std::string> inputs;
        std::vector<std::string> outputs;
        shaders::RuntimeConstants constants;
    };
    struct PostProcessingStage
    {
        std::string name;
        std::vector<std::string> inputs;
        std::vector<std::string> outputs;
        shaders::RuntimeConstants constants;
        PostProcessingStage(const PostProcessingStageCreateInfo& info);
    };
    struct Window;
    struct PostProcessingPipeline
    {
        Window& window;
        KeyIDVector<float, PostProcessingStage> stages;
        PostProcessingPipeline(Window& window);
        KeyIDVector<float, PostProcessingStage>::EmplaceBackTuple addStage(float floatingIndex, const PostProcessingStageCreateInfo& info);
        bool removeStage(size_t id);
    };
}