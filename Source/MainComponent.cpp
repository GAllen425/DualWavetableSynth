/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : juce::AudioAppComponent(otherDeviceManager)
{
	this->setUsingNativeTitleBar(true);
	otherDeviceManager.initialise(2, 2, nullptr, true);
	audioSettingsComp.reset(new AudioDeviceSelectorComponent(otherDeviceManager,0,2,0,2,true,true,true,true));
	addAndMakeVisible(audioSettingsComp.get());

	createWavetable(MainComponent::waveTableShape::NONE,
					"comboBox1");
	createWavetable(MainComponent::waveTableShape::NONE,
					"comboBox2");

	addAndMakeVisible(blendKnob);
	blendKnob.setRange(-0.99, 0.99);
	blendKnob.setSliderStyle(Slider::SliderStyle::RotaryVerticalDrag);
	blendKnob.setNumDecimalPlacesToDisplay(2);
	blendKnob.addListener(this);
	blendKnob.setTextBoxStyle(Slider::TextBoxBelow, false, 120, blendKnob.getTextBoxHeight());

	addAndMakeVisible(waveTableComboBox1);
	waveTableComboBox1.addItemList(waveTableShapeStrings, 1);
	waveTableComboBox1.setComponentID("comboBox1");
	waveTableComboBox1.addListener(this);

	addAndMakeVisible(waveTableComboBox2);
	waveTableComboBox2.addItemList(waveTableShapeStrings, 1);
	waveTableComboBox2.setComponentID("comboBox2");
	waveTableComboBox2.addListener(this);

	drawBuffer1.setBufferToDraw(&waveTable1);
	addAndMakeVisible(drawBuffer1);
	drawBuffer2.setBufferToDraw(&waveTable2);
	addAndMakeVisible(drawBuffer2);

	drawBufferCombined.setBufferToDraw(&finalBuffer);
	addAndMakeVisible(drawBufferCombined);

	setSize(800, 600);
	setAudioChannels(0, 2);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::createWavetable(MainComponent::waveTableShape shape,
	juce::String componentId)
{
	AudioSampleBuffer* waveTablePtr;

	if (componentId == "comboBox1")
	{
		waveTablePtr = &waveTable1;
		initialised1 = true;
		DBG("combo 1");
	}
	else if (componentId == "comboBox2")
	{
		waveTablePtr = &waveTable2;
		initialised2 = true;
		DBG("combo 2");
	}
	else
	{
		DBG("WARNING: Wavetable componentId unspecified");
	}

	waveTablePtr->setSize(1, tableSize + 1);
	waveTablePtr->clear();
	auto* samples = waveTablePtr->getWritePointer(0);

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

	if (initialised1 && initialised2)
	{
		const float* samples1 = waveTable1.getReadPointer(0);
		const float* samples2 = waveTable2.getReadPointer(0);

		finalBuffer.setSize(1, tableSize + 1);
		finalBuffer.clear();
		auto* samplesCombined = finalBuffer.getWritePointer(0);

		for (auto i = 0; i < tableSize; ++i)
		{
			samplesCombined[i] = (fabs(blend - 1.0f)*samples1[i] + fabs(blend + 1.0f)*samples2[i]) / 2.0f;
		}
	}
}

float MainComponent::getWaveTableSample(MainComponent::waveTableShape shape, float angle)
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
		sample = (-2.0f / MathConstants<float>::pi) * std::atan(1.0f / std::tan(angle / 2.0f - MathConstants<float>::pi / 2.0f));
		break;
	default:
		sample = 0.0f;
	}
	return sample;
}
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
	globalSampleRate = sampleRate;
	InitialiseOscillators();
}

void MainComponent::InitialiseOscillators()
{
	oscillators1.clear(true);
	oscillators2.clear(true);
	auto numberOfOscillators = 10;

	for (auto i = 0; i < numberOfOscillators; ++i)
	{
		auto midiNote = 48.0;
		auto frequency = 440.0 * pow(2.0, (midiNote - 69.0) / 12.0);

		if (initialised1)
		{

			WavetableOscillator* oscillator1 = new WavetableOscillator(waveTable1);
			oscillator1->setFrequency((float)frequency, globalSampleRate);
			oscillators1.add(oscillator1);
		}

		if (initialised2)
		{
			WavetableOscillator* oscillator2 = new WavetableOscillator(waveTable2);
			oscillator2->setFrequency((float)frequency, globalSampleRate);
			oscillators2.add(oscillator2);

		}
	}

	// Divide by two as there is two wavetables
	level = 0.25f / numberOfOscillators / 2.0f;
}

void MainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
	auto* leftBuffer = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
	auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

    bufferToFill.clearActiveBufferRegion();

	for (auto oscillatorIndex = 0; oscillatorIndex < oscillators1.size(); ++oscillatorIndex)
	{
		auto* oscillator1 = oscillators1.getUnchecked(oscillatorIndex);
		auto* oscillator2 = oscillators2.getUnchecked(oscillatorIndex);

		for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
		{
			auto levelSample1 = oscillator1->getNextSample() * level;
			auto levelSample2 = oscillator2->getNextSample() * level;

			leftBuffer[sample] += (levelSample1 + levelSample2);
			rightBuffer[sample] += (levelSample1 + levelSample2);
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
	Rectangle<int> dropdownArea = area.removeFromTop(area.getHeight() / 2.0f);
	Rectangle<int> leftDropDownArea = dropdownArea.removeFromLeft(1.0f/3.0f * dropdownArea.getWidth());
	Rectangle<int> blendArea = dropdownArea.removeFromLeft(dropdownArea.getWidth() / 2.0f);

	waveTableComboBox1.setBounds(leftDropDownArea.removeFromTop(leftDropDownArea.getHeight() * 1 / 4).reduced(10));
	drawBuffer1.setBounds(leftDropDownArea);

	blendKnobLabel.setBounds(blendArea.removeFromTop(blendArea.getHeight() * 1 / 4).reduced(10));
	blendKnob.setBounds(blendArea);

	waveTableComboBox2.setBounds(dropdownArea.removeFromTop(dropdownArea.getHeight() * 1 / 4).reduced(10));
	drawBuffer2.setBounds(dropdownArea);

	shapeArea = area.reduced(10);
	drawBufferCombined.setBounds(shapeArea);

}

void MainComponent::comboBoxChanged(ComboBox* comboBox)
{
	if (comboBox == &waveTableComboBox1 || comboBox == &waveTableComboBox2)
	{
		//DBG("waveshape : " + static_cast<waveTableShape>(comboBox->getSelectedId() - 1));
		//DBG("box : " + comboBox->getName());
		createWavetable(static_cast<waveTableShape>(comboBox->getSelectedId() - 1),
			comboBox->getComponentID());

		InitialiseOscillators();
		repaint();
	}
}

void MainComponent::sliderValueChanged(Slider* slider)
{
	if (slider == &blendKnob)
	{
		blend = blendKnob.getValue();
		auto* drawSamplesCombined = finalBuffer.getWritePointer(0);		
		const float* samples1 = waveTable1.getReadPointer(0);
		const float* samples2 = waveTable2.getReadPointer(0);
		for (auto i = 0; i < tableSize; ++i)
		{
			drawSamplesCombined[i] = (fabs(blend - 1.0f)*samples1[i] + fabs(blend + 1.0f)*samples2[i]) / 2.0f;
		
		}

		InitialiseOscillators();
		repaint();
	}
}
