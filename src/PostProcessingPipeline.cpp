#include <zg/PostProcessingPipeline.hpp>
#include <zg/Window.hpp>
using namespace zg;
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
		name(info.name), inputs(info.inputs), outputs(info.outputs), constants(info.constants),
        setShaderConstants(info.setShaderConstants),
        staticOnAttached(info.staticOnAttached), staticOnDetached(info.staticOnDetached)
{
}
PostProcessingPipeline::PostProcessingPipeline(const std::vector<size_t*>& INDEX_STACK) : INDEX_STACK(INDEX_STACK) {}
KeyIDVector<float, PostProcessingStage>::EmplaceBackTuple
PostProcessingPipeline::addStage(float floatingIndex, const PostProcessingStageCreateInfo& info)
{
    auto& window = Registry::getWindow(INDEX_STACK);
	auto emplace_tuple = stages.emplace_back_key(floatingIndex, info);
	auto& stage = *std::get<KEY_ID_VECTOR_VALUE_INDEX>(emplace_tuple);
    stage.floatingIndex = floatingIndex;
	std::vector<textures::Framebuffer::TextureAttachmentPair> attachments;
	for (auto& outputPair : stage.outputs)
	{
		auto& key = outputPair.first;
        stage.constants.push_back(key);
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
		auto textureFilterType = textures::Texture::FilterType::Linear;
		auto texture =
			std::make_shared<textures::Texture>(window.iRenderer, glm::ivec4(window.windowWidth, window.windowHeight, 1, 0),
																					(const void*)0, textureFormat, textureType, textureFilterType, true);
        textureRegistry.registerOutput(floatingIndex, key, texture);
		attachments.push_back({texture, attachmentType});
	}
    for (auto& input : stage.inputs)
        stage.constants.push_back(input);
    auto stageConstantsSize = stage.constants.size();
    auto stageConstantsData = stage.constants.data();
    for (size_t i = 0; i < stageConstantsSize; ++i)
    {
        for (size_t j = i + 1; j < stageConstantsSize; ++j)
        {
            if (stageConstantsData[i] == stageConstantsData[j])
            {
                stage.constants.erase(stage.constants.begin() + j);
                stageConstantsSize = stage.constants.size();
                stageConstantsData = stage.constants.data();
                j--;
            }
        }
    }
    auto framebuffer = std::make_shared<textures::Framebuffer>(window.iRenderer, attachments);
	framebuffers.emplace_back_key(floatingIndex, framebuffer);
    fullscreenQuads.emplace_back_key(floatingIndex, window.INDEX_STACK, stage.constants);
    if (!calledStaticOnAttached[stage.name])
    {
        if (stage.staticOnAttached)
        {
            stage.staticOnAttached();
            calledStaticOnAttached[stage.name] = true;
        }
    }
	return emplace_tuple;
}
bool PostProcessingPipeline::removeStage(size_t id)
{
	auto iter = stages.find_id(id);
	if (iter == stages.end())
		return false;
    auto& stage = *iter;
    if (!calledStaticOnDetached[stage.name])
    {
        if (stage.staticOnDetached)
        {
            stage.staticOnDetached();
            calledStaticOnDetached[stage.name] = true;
        }
    }
	stages.erase(iter);
	return true;
}
void PostProcessingPipeline::cleanup()
{
    textureRegistry.map.clear();
	framebuffers.clear();
    fullscreenQuads.clear();
}
std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>> PostProcessingPipeline::postProcess()
{
    auto key_iter = stages.key_begin();
    auto key_end = stages.key_end();
    auto& textureMap = textureRegistry.map;
    auto texturePairIter = textureMap.begin();
    auto textureMapEnd = textureMap.end(), textureMapBegin = texturePairIter;
    while (key_iter != key_end)
    {
        auto& stage = *key_iter;
        auto& floatingIndex = stage.floatingIndex;
        auto& inputs = stage.inputs;
        auto inputsSize = inputs.size();
        std::vector<std::pair<std::string, std::shared_ptr<textures::Texture>>> inputTextures;
        auto prevTexturePairIter = texturePairIter;
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
                ++texturePairIter;
                if (texturePairIter == textureMapEnd)
                {
                    break;
                }
            }
            else
            {
                break;
            }
        }
        while (inputTextures.size() < inputsSize)
        {
            bool breakThisIteration = (prevTexturePairIter == textureMapBegin);
            auto& texturePair = prevTexturePairIter->first;
            auto existingInputIter = std::find_if(inputTextures.begin(), inputTextures.end(), [&](std::pair<std::string, std::shared_ptr<textures::Texture>>& pair)
            {
                return pair.first == texturePair.second;
            });
            if (existingInputIter == inputTextures.end() && texturePair.first < floatingIndex)
            {
                auto found_iter = std::find_if(inputs.begin(), inputs.end(), [&](auto& val) {
                    return val == texturePair.second;
                });
                if (found_iter != inputs.end())
                {
                    inputTextures.push_back({*found_iter, prevTexturePairIter->second});
                }
            }
            if (breakThisIteration)
            {
                break;
            }
            --prevTexturePairIter;
        }
        auto inputTexturesSize = inputTextures.size();
        auto inputTexturesData = inputTextures.data();
        auto inputsData = inputs.data();
        auto swap = [&](size_t i, size_t j)
        {
            auto s = inputTexturesData[i];
            inputTexturesData[i] = inputTexturesData[j];
            inputTexturesData[j] = s;
        };
        for (size_t i = 0; i < inputTexturesSize; ++i)
        {
            for (size_t j = i + 1; j < inputsSize; j++)
            {
                if (inputsData[j] == inputTexturesData[i].first)
                {
                    swap(i, j);
                    break;
                }
            }
        }
        auto framebufferIter = framebuffers.find_key(stage.floatingIndex);
        auto& framebuffer = **framebufferIter;
        auto fullscreenQuadIter = fullscreenQuads.find_key(stage.floatingIndex);
        assert(fullscreenQuadIter != fullscreenQuads.end());
        auto& fullscreenQuad = *fullscreenQuadIter;
        framebuffer.bind();
        auto& shader = *fullscreenQuad.addShader();
        shader.bind(fullscreenQuad);
        if (stage.setShaderConstants)
        {
            stage.setShaderConstants(shader, fullscreenQuad);
        }
        fullscreenQuad.render(inputTextures, true);
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