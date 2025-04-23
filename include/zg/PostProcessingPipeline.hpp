#pragma once
#include <string>
#include <vector>
#include <map>
#include <zg/textures/Texture.hpp>
#include <zg/shaders/RuntimeConstants.hpp>
#include <zg/KeyIDVector.hpp>
#include <zg/textures/Framebuffer.hpp>
#include <zg/FullscreenQuad.hpp>
namespace std
{
	template <>
	struct hash<std::pair<float, std::string>>
	{
		size_t operator()(const std::pair<float, std::string> &vec) const
		{
			return std::hash<float>{}(vec.first) ^ std::hash<std::string>{}(vec.second);
		}
	};
}
namespace zg
{
    struct TextureOutputRegistry
    {
        static std::map<std::pair<float, std::string>, std::shared_ptr<textures::Texture>> map;
        static void registerOutput(float floatingIndex, const std::string& key, const std::shared_ptr<textures::Texture>& texture);
        static void deregisterOutput(float floatingIndex, const std::string& key);
    };
    struct PostProcessingStageCreateInfo
    {
        std::string name;
        std::vector<std::string> inputs;
        std::vector<std::pair<std::string, textures::Framebuffer::AttachmentType>> outputs;
        shaders::RuntimeConstants constants;
    };
    struct PostProcessingStage
    {
        std::string name;
        float floatingIndex;
        std::vector<std::string> inputs;
        std::vector<std::pair<std::string, textures::Framebuffer::AttachmentType>> outputs;
        shaders::RuntimeConstants constants;
        PostProcessingStage(const PostProcessingStageCreateInfo& info);
    };
    struct Window;
    struct PostProcessingPipeline
    {
        Window& window;
        KeyIDVector<float, PostProcessingStage> stages;
        KeyIDVector<float, textures::Framebuffer*> framebuffers;
        KeyIDVector<float, FullscreenQuad> fullscreenQuads;
        bool dirty = false;
        PostProcessingPipeline(Window& window);
        KeyIDVector<float, PostProcessingStage>::EmplaceBackTuple addStage(float floatingIndex, const PostProcessingStageCreateInfo& info);
        bool removeStage(size_t id);
        void cleanup();
        std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>> postProcess();
    };
}