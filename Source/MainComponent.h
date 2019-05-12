/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "DrawBufferComponent.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/

class WavetableOscillator
{
public:
	WavetableOscillator(const AudioSampleBuffer& wavetableToUse)
		: wavetableOscBuffer(wavetableToUse),
		  tableSize (wavetableOscBuffer.getNumSamples() - 1)
	{
		jassert(wavetableOscBuffer.getNumChannels() == 1);
	}

	void setFrequency(float frequency, float sampleRate);
	forcedinline float getNextSample() noexcept;



private:
	const AudioSampleBuffer& wavetableOscBuffer;
	const int tableSize;
	float currentIndex = 0.0, tableDelta = 0.0f;
};

class MainComponent   : public AudioAppComponent,
						public ComboBox::Listener
{
public:
    //==============================================================================
    MainComponent();
	~MainComponent();

	enum waveTableShape
	{
		SINE,
		SQUARE,
		SAW,
		NONE
	};

	juce::StringArray waveTableShapeStrings { "Sine", "Square", "Saw", "None" };

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

	void createWavetable (MainComponent::waveTableShape shape);
	float getWaveTableSample(MainComponent::waveTableShape shape, float angle);
	void MainComponent::InitialiseOscillators();

    //==============================================================================
    void paint (Graphics& g) override;

    void resized() override;
	
	void comboBoxChanged(ComboBox* comboBox) override
	{
		if (comboBox == &waveTableComboBox)
		{
			DBG("Selected ID: " + static_cast<waveTableShape>(comboBox->getSelectedId() - 1));
			DBG("Corresponding string: " + waveTableShapeStrings[static_cast<waveTableShape>(comboBox->getSelectedId() - 1)]);
			createWavetable(static_cast<waveTableShape>(comboBox->getSelectedId()-1));
			InitialiseOscillators();
			repaint();

		}
	}

private:
    //==============================================================================
    // Your private member variables go here...
	Label waveTableShapeLabel;
	ComboBox waveTableComboBox;
	double globalSampleRate;
	

	// Wavetable
	const unsigned int tableSize = 1 << 7;
	float level = 0.0f;

	AudioSampleBuffer waveTable;
	OwnedArray<WavetableOscillator> oscillators;
	
	DrawBufferComponent drawBufferComponent;
	Rectangle<int> shapeArea;




    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
