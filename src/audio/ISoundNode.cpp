#include <zg/audio/AudioEngine.hpp>
using namespace zg::audio;
ISoundNode::ISoundNode(AudioEngine& _audioEngine, const bool& isOutput, const bool& isInput) :
		audioEngine(_audioEngine), isOutput(isOutput), isInput(isInput)
{
}
void ISoundNode::activate()
{
	activationTime = audioEngine.getEngineTime();
	isActive = activateVirtual();
}
bool ISoundNode::activateVirtual()
{
	return true;
}
void ISoundNode::deactivate()
{
	deactivationTime = audioEngine.getEngineTime();
	isActive = deactivateVirtual();
}
bool ISoundNode::deactivateVirtual()
{
	return false;
}