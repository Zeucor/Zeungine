#include <zg/audio/AudioStage.hpp>
using namespace zg::audio;
AudioStage::AudioStage(AudioEngine& _audioEngine) : ISoundNode(_audioEngine, true, true){}
std::vector<float> AudioStage::inputFrames(const float *frames, const int32_t &channelCount, const unsigned long &frameCount, const audio_time_t &time)
{
	std::lock_guard lock(soundNodesMutex);
	std::vector<float> mixedFrames(frameCount * channelCount, 0.0f);
	for (auto &soundNodePair : soundNodes)
	{
		auto &soundNode = *soundNodePair.second.get();
		if (soundNode.isInput)
		{
			std::vector<float> soundNodeFrames(frameCount * channelCount, 0.0f);
			soundNodeFrames = soundNode.inputFrames(frames, channelCount, frameCount, time);
			for (size_t i = 0; i < mixedFrames.size(); ++i)
			{
				mixedFrames[i] += soundNodeFrames[i];
			}
		}
	}
	return mixedFrames;
}
void AudioStage::outputFrames(float *frames, const int32_t &channelCount, const unsigned long &frameCount, const audio_time_t &time)
{
	std::lock_guard lock(soundNodesMutex);
	std::vector<float> mixedFrames(frameCount * channelCount, 0.0f);
	auto mixedFramesSize = mixedFrames.size();
	for (auto &soundNodePair : soundNodes)
	{
		auto &soundNode = *soundNodePair.second.get();
		if (soundNode.isOutput)
		{
			std::vector<float> soundNodeFrames(frames, frames + (frameCount * channelCount));
			soundNode.outputFrames(soundNodeFrames.data(), channelCount, frameCount, time);
			for (size_t i = 0; i < mixedFramesSize; ++i)
			{
				mixedFrames[i] += soundNodeFrames[i];
			}
		}
	}
	for (size_t i = 0; i < mixedFramesSize; i++)
	{
		frames[i] += mixedFrames[i];
	}
}
uint64_t AudioStage::addSoundNode(const std::shared_ptr<ISoundNode> &soundNodePointer, int64_t index)
{
	std::lock_guard lock(soundNodesMutex);
	++soundNodesCount;
	if (index == -1)
	{
		index = soundNodesCount;
	}
	soundNodes[index] = soundNodePointer;
	return index;
}
bool AudioStage::removeSoundNode(uint64_t& index)
{
	std::lock_guard lock(soundNodesMutex);
	auto soundNodeIter = soundNodes.find(index);
	if (soundNodeIter != soundNodes.end())
	{
		soundNodes.erase(soundNodeIter);
		index = 0;
		return true;
	}
	return false;
}