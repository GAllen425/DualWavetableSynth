/*
  ==============================================================================

    DrawBufferComponent.cpp
    Created: 27 Apr 2019 3:47:04pm
    Author:  The Big G

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "DrawBufferComponent.h"

//==============================================================================
DrawBufferComponent::DrawBufferComponent()
{

}

DrawBufferComponent::~DrawBufferComponent()
{
}

void DrawBufferComponent::setBufferToDraw(AudioSampleBuffer* bufferToDraw)
{
	drawBuffer = bufferToDraw;
}

void DrawBufferComponent::paint (Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (Colours::white);

	if (drawBuffer != nullptr)
	{
		float prevSample = 0.0f;
		float scaleX = getWidth() / drawBuffer->getNumSamples();
		float scaleY = getHeight() / 4;

		const float* samples = drawBuffer->getReadPointer(0);


		Path path;
		path.startNewSubPath(0, getHeight() / 2.0f);

		for (int i = 1; i < drawBuffer->getNumSamples(); i++)
		{
			path.lineTo(i*scaleX, getHeight() / 2.0f - samples[i] * scaleY);
		}

		g.strokePath(path, PathStrokeType(1.3f));
	}

}

void DrawBufferComponent::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
