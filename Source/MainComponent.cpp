/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

void WavetableOscillator::setFrequency(float frequency, float sampleRate)
{
	auto tableSizeOverSampleRate = tableSize / sampleRate;
	tableDelta = frequency * tableSizeOverSampleRate;
}

forcedinline float WavetableOscillator::getNextSample() noexcept
{
	auto index0 = (unsigned int)currentIndex;
	auto index1 = index0  + 1;

	auto frac = currentIndex - (float)index0;

	auto* table = wavetableOscBuffer.getReadPointer(0);
	auto value0 = table[index0];
	auto value1 = table[index1];

	auto currentSample = value0 + frac * (value1 - value0);

	if ((currentIndex += tableDelta) > tableSize)
		currentIndex -= tableSize;

	return currentSample;
}

void MainComponent::createWavetable (MainComponent::waveTableShape shape)
{
	waveTable.setSize(1, tableSize + 1);
	waveTable.clear();
	auto* samples = waveTable.getWritePointer(0);

	int harmonics[] = { 1, 1, 1, 1, 1 };
	float harmonicWeights[] = { 0.5f, 0.1f, 0.05f, 0.0125f, 0.005f };

	jassert(numElementsInArray(harmonics) == numElementsInArray(harmonicWeights));

	for (auto harmonic = 0; harmonic < numElementsInArray(harmonics); ++harmonic)
	{
		auto angleDelta = MathConstants<double>::twoPi / (double)(tableSize - 1) * harmonics[harmonic];
		auto currentAngle = 0.0;

		for (auto i = 0; i < tableSize; ++i)
		{
			auto sample = getWaveTableSample(shape, currentAngle);
			samples[i] += (float)sample * harmonicWeights[harmonic];
			currentAngle += angleDelta;
		}
	}
	samples[tableSize] = samples[0];
}

float MainComponent::getWaveTableSample (MainComponent::waveTableShape shape, float angle)
{
	float sample = 0.0f;

	switch (shape) {
	case SINE:
		sample = std::sin(angle);
		break;
	case SQUARE:
	{
		float sinOfAngle = std::sin(angle);
		if (sinOfAngle < 0)
		{
			sample = -1.0f;
		}
		else if (-0.001f <= sinOfAngle && sinOfAngle <= 0.001f)
		{
			sample = 0.0f;
		}
		else
		{
			sample = 1.0f;
		}
		break;
	}
	case SAW:
		sample = (-2.0f / MathConstants<float>::pi) * std::atan(1.0f / std::tan(angle/2.0f - MathConstants<float>::pi / 2.0f));
		break;
	default:
		sample = 0.0f;
	}
	return sample;
}

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.

	// Start up as sine
	createWavetable(MainComponent::waveTableShape::NONE);

	addAndMakeVisible(waveTableComboBox);
	waveTableComboBox.addItemList(waveTableShapeStrings, 1);
	waveTableComboBox.addListener(this);

	addAndMakeVisible(waveTableShapeLabel);
	waveTableShapeLabel.setText("Wave Shape", dontSendNotification);
	waveTableShapeLabel.attachToComponent(&waveTableComboBox, true);

	drawBufferComponent.setBufferToDraw(&waveTable);
	addAndMakeVisible(drawBufferComponent);

	setSize(800, 600);
	setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
	globalSampleRate = sampleRate;
	InitialiseOscillators();
}

void MainComponent::InitialiseOscillators()
{
	oscillators.clear(true);
	auto numberOfOscillators = 10;

	for (auto i = 0; i < numberOfOscillators; ++i)
	{
		auto* oscillator = new WavetableOscillator(waveTable);

		auto midiNote = 48.0;
		auto frequency = 440.0 * pow(2.0, (midiNote - 69.0) / 12.0);

		oscillator->setFrequency((float)frequency, globalSampleRate);
		oscillators.add(oscillator);
	}

	level = 0.25f / numberOfOscillators;
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
	auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
	auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    bufferToFill.clearActiveBufferRegion();

	for (auto oscillatorIndex = 0; oscillatorIndex < oscillators.size(); ++oscillatorIndex)
	{
		auto* oscillator = oscillators.getUnchecked(oscillatorIndex);

		for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
		{
			auto levelSample = oscillator->getNextSample() * level;

			leftBuffer[sample] += levelSample;
			rightBuffer[sample] += levelSample;
		}
	}
}

void MainComponent::releaseResources()
{

}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
	const int border = 10;
	Rectangle<int> area = getLocalBounds();
	Rectangle<int> dropdownArea = area.removeFromTop(area.getHeight() / 4);

	waveTableComboBox.setBounds(dropdownArea.removeFromBottom(dropdownArea.getHeight() / 2).reduced(10));
	waveTableShapeLabel.setBounds(dropdownArea.reduced(10));

	shapeArea = area.reduced(10);
	drawBufferComponent.setBounds(shapeArea);

}
