/*
  ==============================================================================

    WaveTableOscillator.cpp
    Created: 30 Jun 2019 11:28:58am
    Author:  The Big G

  ==============================================================================
*/

#include "WaveTableOscillator.h"

void WavetableOscillator::setFrequency(float frequency, float sampleRate)
{
	auto tableSizeOverSampleRate = tableSize / sampleRate;
	tableDelta = frequency * tableSizeOverSampleRate;
}

float WavetableOscillator::getNextSample() noexcept
{
	auto index0 = (unsigned int)currentIndex;
	auto index1 = index0 + 1;

	auto frac = currentIndex - (float)index0;

	auto* table = wavetableOscBuffer.getReadPointer(0);
	auto value0 = table[index0];
	auto value1 = table[index1];

	auto currentSample = value0 + frac * (value1 - value0);

	if ((currentIndex += tableDelta) > tableSize)
		currentIndex -= tableSize;

	return currentSample;
}