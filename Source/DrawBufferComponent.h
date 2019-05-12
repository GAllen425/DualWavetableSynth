/*
  ==============================================================================

    DrawBufferComponent.h
    Created: 27 Apr 2019 3:47:04pm
    Author:  The Big G

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class DrawBufferComponent    : public Component
{
public:
    DrawBufferComponent();
    ~DrawBufferComponent();

    void paint (Graphics&) override;
    void resized() override;
	void setBufferToDraw(AudioSampleBuffer* bufferToDraw);

private:
	AudioSampleBuffer* drawBuffer;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrawBufferComponent)
};
