#pragma once
#include <string>
#include <vector>
#include <map>
#include <zg/textures/Texture.hpp>
#include <zg/shaders/RuntimeConstants.hpp>
#include <zg/KeyIDVector.hpp>
#include <zg/textures/Framebuffer.hpp>
#include <zg/FullscreenQuad.hpp>
#include <zg/shaders/Shader.hpp>
#include <zg/vaos/VAO.hpp>
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
        std::map<std::pair<float, std::string>, std::shared_ptr<textures::Texture>> map;
        void registerOutput(float floatingIndex, const std::string& key, const std::shared_ptr<textures::Texture>& texture);
        void deregisterOutput(float floatingIndex, const std::string& key);
    };
    using P3OutputsVector = std::vector<std::tuple<std::string, textures::Framebuffer::AttachmentType, textures::Texture::AddressMode>>;
    struct PostProcessingStageCreateInfo
    {
        std::string name;
        std::vector<std::string> inputs;
        P3OutputsVector outputs;
        shaders::RuntimeConstants constants;
        std::function<void(zg::shaders::Shader&, zg::vaos::VAO&)> setShaderConstants;
        std::function<void()> staticOnAttached;
        std::function<void()> staticOnDetached;
    };
    struct PostProcessingStage
    {
        std::string name;
        float floatingIndex;
        std::vector<std::string> inputs;
        P3OutputsVector outputs;
        shaders::RuntimeConstants constants;
        std::function<void(zg::shaders::Shader&, zg::vaos::VAO&)> setShaderConstants;
        std::function<void()> staticOnAttached;
        std::function<void()> staticOnDetached;
        PostProcessingStage(const PostProcessingStageCreateInfo& info);
    };
    struct Window;
    struct PostProcessingPipeline
    {
        TextureOutputRegistry textureRegistry;
        std::vector<size_t*> INDEX_STACK;
        KeyIDVector<float, PostProcessingStage> stages;
        KeyIDVector<float, std::shared_ptr<textures::Framebuffer>> framebuffers;
        KeyIDVector<float, FullscreenQuad> fullscreenQuads;
        std::unordered_map<std::string, bool> calledStaticOnAttached;
        std::unordered_map<std::string, bool> calledStaticOnDetached;
        bool dirty = false;
        PostProcessingPipeline(const std::vector<size_t*>& INDEX_STACK);
        KeyIDVector<float, PostProcessingStage>::EmplaceBackTuple addStage(float floatingIndex, const PostProcessingStageCreateInfo& info);
        bool removeStage(size_t id);
        void cleanup();
        std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>> postProcess();
    };
}