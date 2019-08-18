/*
  ==============================================================================

    WaveTableOscillator.h
    Created: 30 Jun 2019 11:28:58am
    Author:  The Big G

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class WavetableOscillator
{
public:
	WavetableOscillator(const AudioSampleBuffer& wavetableToUse)
		: wavetableOscBuffer(wavetableToUse),
		tableSize(wavetableOscBuffer.getNumSamples() - 1)
	{
		jassert(wavetableOscBuffer.getNumChannels() == 1);
	}

	void setFrequency(float frequency, float sampleRate);
	float getNextSample() noexcept;



private:
	const AudioSampleBuffer& wavetableOscBuffer;
	const int tableSize;
	float currentIndex = 0.0, tableDelta = 0.0f;
};