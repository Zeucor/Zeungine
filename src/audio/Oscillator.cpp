#include <zg/audio/Oscillator.hpp>
#include <zg/GlobalUID.hpp>
using namespace zg::audio;
float Oscillator::sineWave(double currentPhase, double baseAmplitude, const WaveParameters& params)
{
	double currentAmplitude = baseAmplitude * params.amplitudeMultiplier;
	return static_cast<float>(currentAmplitude * std::sin(currentPhase + params.phaseOffset));
}
float Oscillator::squareWave(double currentPhase, double baseAmplitude, const WaveParameters& params)
{
	double currentAmplitude = baseAmplitude * params.amplitudeMultiplier;
	double normalizedPhase = fmod((currentPhase + params.phaseOffset) / (2.0 * ZG_PI), 1.0);
	if (normalizedPhase < 0)
		normalizedPhase += 1.0;

	double pulseWidth = std::clamp(params.pulseWidth, 0.0, 1.0);

	return static_cast<float>(currentAmplitude * (normalizedPhase < pulseWidth ? 1.0 : -1.0));
}
float Oscillator::sawtoothWave(double currentPhase, double baseAmplitude, const WaveParameters& params)
{
	double currentAmplitude = baseAmplitude * params.amplitudeMultiplier;
	// Apply phase offset and normalize phase to [0, 1) for shape calculation
	double normalizedPhase = fmod((currentPhase + params.phaseOffset) / (2.0 * ZG_PI), 1.0);
	if (normalizedPhase < 0)
		normalizedPhase += 1.0;

	// A simpler linear sawtooth: rises linearly from -1 to 1
	double value = -1.0 + 2.0 * normalizedPhase;

	// Asymmetry implementation (can be complex, using a simpler linear blend approach for now)
	double asymmetry = std::clamp(params.asymmetry, -1.0, 1.0);
	if (asymmetry != 0.0)
	{
		// Simple linear interpolation based on asymmetry
		// More complex asymmetry would involve piecewise functions
		if (normalizedPhase < (1.0 + asymmetry) / 2.0)
		{
			value = -1.0 + 2.0 * normalizedPhase / ((1.0 + asymmetry) / 2.0);
		}
		else
		{
			value = 1.0 - 2.0 * (normalizedPhase - (1.0 + asymmetry) / 2.0) / ((1.0 - asymmetry) / 2.0);
		}
	}


	return static_cast<float>(currentAmplitude * value);
}
float Oscillator::triangleWave(double currentPhase, double baseAmplitude, const WaveParameters& params)
{
	double currentAmplitude = baseAmplitude * params.amplitudeMultiplier;
	// Apply phase offset and normalize phase to [0, 1) for shape calculation
	double normalizedPhase = fmod((currentPhase + params.phaseOffset) / (2.0 * ZG_PI), 1.0);
	if (normalizedPhase < 0)
		normalizedPhase += 1.0;

	// Asymmetry parameter shifts the peak location
	double asymmetry = std::clamp(params.asymmetry, -1.0, 1.0); // -1: peak at start, 1: peak at end, 0: peak at 0.5
	double peakPos = 0.5 + asymmetry * 0.5; // Peak position in [0, 1]

	double value;
	if (normalizedPhase < peakPos)
	{
		value = -1.0 + 2.0 * (normalizedPhase / peakPos);
	}
	else
	{
		value = 1.0 - 2.0 * ((normalizedPhase - peakPos) / (1.0 - peakPos));
	}

	// Handle edge case where peakPos is exactly 0 or 1
	if (peakPos == 0.0 && normalizedPhase != 0.0)
		value = -1.0; // At 0, value is 1, otherwise -1
	if (peakPos == 1.0 && normalizedPhase != 1.0)
		value = -1.0; // At 1, value is 1, otherwise -1
	if (peakPos == 0.0 && normalizedPhase == 0.0)
		value = 1.0;
	if (peakPos == 1.0 && normalizedPhase == 1.0)
		value = 1.0;


	return static_cast<float>(currentAmplitude * value);
}
Oscillator::Oscillator(AudioEngine& _audioEngine, double amp) :
		ISoundNode(_audioEngine, true, false), amplitude(amp)
{
}
size_t Oscillator::addWaveLayer(WaveGeneratorFunc func, const WaveParameters& params)
{
    auto ID = GlobalUID::GetNew();
	layeredWaves[ID] = {func, params};
    return ID;
}
bool Oscillator::removeWaveLayer(size_t& ID)
{
    auto iter = layeredWaves.find(ID);
    if (iter == layeredWaves.end())
        return false;
    layeredWaves.erase(iter);
    ID = 0;
    return true;
}
bool Oscillator::modifyWaveLayerParams(size_t index, const WaveParameters& newParams)
{
	if (index < layeredWaves.size())
	{
		layeredWaves[index].params = newParams;
		return true;
	}
	return false;
}
std::vector<float> Oscillator::inputFrames(const float* frames, const int32_t& channelCount,
																					 const unsigned long& frameCount, const audio_time_t& time)
{
	return {};
}
void Oscillator::outputFrames(float* frames, const int32_t& channelCount, const unsigned long& frameCount,
															const audio_time_t& time)
{
	bool currentlyProcessing = isActive || (currentEnvelopeLevel > 0.0 && envelopeTargetLevel == 0.0);

	if (!currentlyProcessing)
	{
		// If not processing, output silence and reset envelope
		std::fill(frames, frames + frameCount * channelCount, 0.0f);
		currentEnvelopeLevel = 0.0;
		envelopeTargetLevel = 0.0;
		envelopeStepPerSample = 0.0;
		return;
	}

	double sampleRate = audioEngine.sampleRate;

	for (unsigned long i = 0; i < frameCount; ++i)
	{
		float sampleValue = 0.0f;

		for (auto& wavePair : layeredWaves)
		{
			auto& wave = wavePair.second;
			double layerFrequency = wave.params.frequency * wave.params.frequencyMultiplier;
			double layerPhaseIncrement = (layerFrequency / sampleRate) * 2.0 * ZG_PI;

			// Generate sample using the layer's current phase
			sampleValue += wave.func(wave.currentPhase, amplitude, wave.params);

			// Increment the layer's phase for the next sample
			wave.currentPhase += layerPhaseIncrement;

			// Optional: Wrap phase to keep it within [0, 2*ZG_PI)
			// This prevents the phase variable from growing infinitely large
			// and avoids potential floating-point issues with very large numbers.
			// Using fmod is fine here as the sample value was already calculated
			// BEFORE wrapping.
			wave.currentPhase = fmod(wave.currentPhase, 2.0 * ZG_PI);
			// Ensure phase is not negative after fmod if the input was negative
			if (wave.currentPhase < 0)
			{
				wave.currentPhase += 2.0 * ZG_PI;
			}
		}

		// Apply the current envelope level to the summed sample value
		sampleValue *= static_cast<float>(currentEnvelopeLevel);

		// Update the envelope level for the next sample
		currentEnvelopeLevel += envelopeStepPerSample;

		// Clamp the envelope level between 0.0 and 1.0
		currentEnvelopeLevel = std::clamp(currentEnvelopeLevel, 0.0, 1.0);

		// If fading out and envelope hits zero, mark as inactive
		if (envelopeTargetLevel == 0.0 && currentEnvelopeLevel <= 0.0)
		{
			deactivate();
			currentEnvelopeLevel = 0.0; // Ensure it's exactly zero
			envelopeStepPerSample = 0.0; // Stop changing
		}
		// If fading in and envelope hits one, ensure it's exactly one and stop changing
		if (envelopeTargetLevel == 1.0 && currentEnvelopeLevel >= 1.0)
		{
			currentEnvelopeLevel = 1.0;
			envelopeStepPerSample = 0.0;
		}


		// Apply base amplitude and final clamping
		sampleValue = std::clamp(sampleValue, -1.0f, 1.0f);

		// Write the sample to all channels
		for (int32_t j = 0; j < channelCount; ++j)
		{
			frames[i * channelCount + j] = sampleValue;
		}
	}
}
bool Oscillator::activateVirtual()
{
	if (currentEnvelopeLevel < 1.0)
	{
		envelopeTargetLevel = 1.0;
		// Calculate step size based on remaining distance and duration
		double remainingDistance = 1.0 - currentEnvelopeLevel;
		double durationSamples = envelopeDuration * audioEngine.sampleRate;
		if (durationSamples > 0)
		{
			envelopeStepPerSample = remainingDistance / durationSamples;
		}
		else
		{
			envelopeStepPerSample = 1.0; // Instantaneous if duration is zero
		}
	}
	return true;
}
bool Oscillator::deactivateVirtual()
{
	// Only start deactivation if not already fully inactive
	if (currentEnvelopeLevel > 0.0)
	{
		envelopeTargetLevel = 0.0;
		// Calculate step size based on current level and duration
		double remainingDistance = currentEnvelopeLevel;
		double durationSamples = envelopeDuration * audioEngine.sampleRate;
		if (durationSamples > 0)
		{
			// Step is negative for fade out
			envelopeStepPerSample = -remainingDistance / durationSamples;
		}
		else
		{
			envelopeStepPerSample = -1.0; // Instantaneous if duration is zero
		}
		return true;
	}
	else
	{
		return false;
	}
}
