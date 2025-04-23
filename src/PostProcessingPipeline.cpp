#include <zg/PostProcessingPipeline.hpp>
#include <zg/Window.hpp>
using namespace zg;
std::map<std::pair<float, std::string>, std::shared_ptr<textures::Texture>> TextureOutputRegistry::map;
void TextureOutputRegistry::registerOutput(float floatingIndex, const std::string& key,
																					 const std::shared_ptr<textures::Texture>& texture)
{
	map.try_emplace({floatingIndex, key}, texture);
}
void TextureOutputRegistry::deregisterOutput(float floatingIndex, const std::string& key)
{
	auto iter = map.find({floatingIndex, key});
	if (iter == map.end())
		return;
	map.erase(iter);
}
PostProcessingStage::PostProcessingStage(const PostProcessingStageCreateInfo& info) :
		name(info.name), inputs(info.inputs), outputs(info.outputs), constants(info.constants)
{
}
PostProcessingPipeline::PostProcessingPipeline(Window& window) : window(window) {}
KeyIDVector<float, PostProcessingStage>::EmplaceBackTuple
PostProcessingPipeline::addStage(float floatingIndex, const PostProcessingStageCreateInfo& info)
{
	auto emplace_tuple = stages.emplace_back_key(floatingIndex, info);
	auto& stage = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(emplace_tuple);
    stage.floatingIndex = floatingIndex;
	std::vector<textures::Framebuffer::TextureAttachmentPair> attachments;
	for (auto& outputPair : stage.outputs)
	{
		auto& key = outputPair.first;
		auto& type = outputPair.second;
		textures::Texture::Format textureFormat;
		textures::Texture::Type textureType;
		textures::Framebuffer::AttachmentType attachmentType;
		switch (type)
		{
		case textures::Framebuffer::AttachmentType::Color:
			{
				textureFormat = textures::Texture::Format::RGBA8;
				textureType = textures::Texture::Type::UnsignedByte;
				attachmentType = textures::Framebuffer::AttachmentType::Color;
				break;
			};
		case textures::Framebuffer::AttachmentType::Depth:
			{
				textureFormat = textures::Texture::Format::Depth;
				textureType = textures::Texture::Type::Float;
				attachmentType = textures::Framebuffer::AttachmentType::Depth;
				break;
			};
		}
		auto textureFilterType = textures::Texture::FilterType::Nearest;
		auto texture =
			std::make_shared<textures::Texture>(window.iRenderer, glm::ivec4(window.windowWidth, window.windowHeight, 0, 0),
																					(const void*)0, textureFormat, textureType, textureFilterType, true);
		TextureOutputRegistry::registerOutput(floatingIndex, key, texture);
		attachments.push_back({texture.get(), attachmentType});
	}
    for (auto& input : stage.inputs)
        stage.constants.push_back(input);
	framebuffers.emplace_back_key(floatingIndex, new textures::Framebuffer(window.iRenderer, attachments));
    fullscreenQuads.emplace_back_key(floatingIndex, window.iRenderer, stage.constants);
	return emplace_tuple;
}
bool PostProcessingPipeline::removeStage(size_t id)
{
	auto iter = stages.find_id(id);
	if (iter == stages.end())
		return false;
	stages.erase(iter);
	return true;
}
void PostProcessingPipeline::cleanup()
{
    TextureOutputRegistry::map.clear();
	auto framebuffersSize = framebuffers.size();
	auto framebuffersData = framebuffers.data();
	for (size_t index = 0; index < framebuffersSize; ++index)
	{
		delete framebuffersData[index];
	}
	framebuffers.clear();
    fullscreenQuads.clear();
}
std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>> PostProcessingPipeline::postProcess()
{
    auto key_iter = stages.key_begin();
    auto key_end = stages.key_end();
    auto& textureMap = TextureOutputRegistry::map;
    auto texturePairIter = textureMap.begin();
    auto textureMapEnd = textureMap.end();
    while (key_iter != key_end)
    {
        auto& stage = *key_iter;
        auto& floatingIndex = stage.floatingIndex;
        auto& inputs = stage.inputs;
        auto inputsSize = inputs.size();
        std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>> inputTextures;
        while (inputTextures.size() < inputsSize)
        {
            auto& texturePair = texturePairIter->first;
            if (texturePair.first < floatingIndex)
            {
                auto found_iter = std::find_if(inputs.begin(), inputs.end(), [&](auto& val) {
                    return val == texturePair.second;
                });
                if (found_iter != inputs.end())
                {
                    inputTextures.push_back({*found_iter, texturePairIter->second});
                }
            }
            ++texturePairIter;
            if (texturePairIter == textureMapEnd)
            {
                break;
            }
        }
        auto framebufferIter = framebuffers.find_key(stage.floatingIndex);
        auto& framebuffer = **framebufferIter;
        auto fullscreenQuadIter = fullscreenQuads.find_key(stage.floatingIndex);
        assert(fullscreenQuadIter != fullscreenQuads.end());
        framebuffer.bind();
        fullscreenQuadIter->render(inputTextures);
        framebuffer.unbind();
        ++key_iter;
        while (texturePairIter != textureMapEnd)
        {
            if (texturePairIter->first.first >= stage.floatingIndex)
                break;
            ++texturePairIter;
        }
    }
    std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>> finalInputs;
    while (texturePairIter != textureMapEnd)
    {
        auto& texturePair = texturePairIter->first;
        auto& texture = texturePairIter->second;
        finalInputs.push_back({ texturePair.second, texture });
        ++texturePairIter;
    }
    return finalInputs;
}