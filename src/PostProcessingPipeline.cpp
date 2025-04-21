#include <zg/PostProcessingPipeline.hpp>
using namespace zg;
std::unordered_map<std::string, std::shared_ptr<textures::Texture>> TextureOutputRegistry::map;
void TextureOutputRegistry::registerOutput(const std::string& key, std::shared_ptr<textures::Texture>& texture)
{
    map.try_emplace(key, texture);
}
void TextureOutputRegistry::deregisterOutput(const std::string& key)
{
    auto iter = map.find(key);
    if (iter == map.end())
        return;
    map.erase(iter);
}
PostProcessingStage::PostProcessingStage(const PostProcessingStageCreateInfo& info):
    name(info.name),
    inputs(info.inputs),
    outputs(info.outputs),
    constants(info.constants)
{}
PostProcessingPipeline::PostProcessingPipeline(Window& window):
    window(window)
{}
KeyIDVector<float, PostProcessingStage>::EmplaceBackTuple PostProcessingPipeline::addStage(float floatingIndex, const PostProcessingStageCreateInfo& info)
{
    return stages.emplace_back_key(floatingIndex, info);
}
bool PostProcessingPipeline::removeStage(size_t id)
{
    auto iter = stages.find_id(id);
    if (iter == stages.end())
        return false;
    stages.erase(iter);
    return true;
}