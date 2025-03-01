#include <zg/audio/AudioPipeline.hpp>
using namespace zg::audio;
AudioPipeline::AudioPipeline(AudioEngine& _audioEngine) : ISoundNode(_audioEngine, true, true){}
std::vector<float> AudioPipeline::inputFrames(const float *frames, const int32_t &channelCount, const unsigned long &frameCount, const audio_time_t &time)
{
	std::lock_guard lock(stagesMutex);
	auto totalNumFrames = frameCount * channelCount;
	std::vector<float> vectorFrames(frames, frames + totalNumFrames);
	for (auto &stagePair : stages)
	{
		auto &stage = *stagePair.second.get();
		if (stage.isInput)
		{
			vectorFrames = stage.inputFrames(vectorFrames.data(), channelCount, frameCount, time);
		}
	}
	return vectorFrames;
}
void AudioPipeline::outputFrames(float *frames, const int32_t &channelCount, const unsigned long &frameCount, const audio_time_t &time)
{
	std::lock_guard lock(stagesMutex);
	for (auto &stagePair : stages)
	{
		auto &stage = *stagePair.second.get();
		if (stage.isOutput)
		{
			stage.outputFrames(frames, channelCount, frameCount, time);
		}
	}
}
uint64_t AudioPipeline::addStage(const std::shared_ptr<AudioStage> &stagePointer, int64_t index)
{
	std::lock_guard lock(stagesMutex);
	++stagesCount;
	if (index == -1)
	{
		index = stagesCount;
	}
	stages[index] = stagePointer;
	return index;
}
bool AudioPipeline::removeStage(uint64_t& index)
{
	std::lock_guard lock(stagesMutex);
	auto stageIter = stages.find(index);
	if (stageIter != stages.end())
	{
		stages.erase(stageIter);
		index = 0;
		return true;
	}
	return false;
}