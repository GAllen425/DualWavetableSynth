/*
  ==============================================================================

    2ndOrderLpfHpf.cpp
    Created: 18 Aug 2019 1:56:47pm
    Author:  The Big G

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "SecondOrderLpfHpf.h"



SecondOrderLpfHpf::SecondOrderLpfHpf()
{
	for (int i = 0; i < FILTER_RESPONSE_RESOLUTION; i++)
	{
		freqResponse[i] = 0.0f;
	}

	addAndMakeVisible(cutoffFreqSlider);
	cutoffFreqSlider.setRange(0, 10000);
	cutoffFreqSlider.setSkewFactorFromMidPoint(2000.0f);
	cutoffFreqSlider.setTextValueSuffix(" Hz");
	cutoffFreqSlider.setNumDecimalPlacesToDisplay(0);
	cutoffFreqSlider.addListener(this);
	cutoffFreqSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
	cutoffFreqSlider.setTextBoxStyle(
		Slider::TextBoxBelow, false, getLocalBounds().getWidth(), cutoffFreqSlider.getTextBoxHeight());

	addAndMakeVisible(QSlider);
	QSlider.setRange(0, 20);
	QSlider.setSkewFactorFromMidPoint(2.0f);
	QSlider.setNumDecimalPlacesToDisplay(2);
	QSlider.addListener(this);
	QSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);

}

SecondOrderLpfHpf::~SecondOrderLpfHpf()
{
	
}

void SecondOrderLpfHpf::calculateCoefficients()
{
	coefficients[filterCoefficients::thetaC] = 2.0f * MathConstants<float>::pi * freqC / sampleRate;
	coefficients[filterCoefficients::d] = 1.0f / Q;
	coefficients[filterCoefficients::beta] = 0.5f * (1.0f - coefficients[filterCoefficients::d] / 2.0f * std::sin(coefficients[filterCoefficients::thetaC]))/
													(1.0f + coefficients[filterCoefficients::d] / 2.0f * std::sin(coefficients[filterCoefficients::thetaC]));
	coefficients[filterCoefficients::gamma] = (0.5f + coefficients[filterCoefficients::beta])*std::cos(coefficients[filterCoefficients::thetaC]);

	if (filterType == filterType::LPF)
	{
		// LPF
		coefficients[filterCoefficients::a0] = (0.5f + coefficients[filterCoefficients::beta] - coefficients[filterCoefficients::gamma]) / 2.0f;
		coefficients[filterCoefficients::a1] = 2.0f * coefficients[filterCoefficients::a0];
		coefficients[filterCoefficients::a0] = coefficients[filterCoefficients::a2];
	}
	else
	{
		// HPF
		coefficients[filterCoefficients::a0] = (0.5f + coefficients[filterCoefficients::beta] + coefficients[filterCoefficients::gamma]) / 2.0f;
		coefficients[filterCoefficients::a1] = -(0.5f + coefficients[filterCoefficients::beta] + coefficients[filterCoefficients::gamma]);
		coefficients[filterCoefficients::a2] = (0.5f + coefficients[filterCoefficients::beta] + coefficients[filterCoefficients::gamma]) / 2.0f;
	}

	coefficients[filterCoefficients::b1] = -2.0f * coefficients[filterCoefficients::gamma];
	coefficients[filterCoefficients::b2] =  2.0f * coefficients[filterCoefficients::beta];
	coefficients[filterCoefficients::c0] = 1.0f;
	coefficients[filterCoefficients::d0] = 0.0f;
}

float SecondOrderLpfHpf::calculateOutput(float xn)
{
	calculateCoefficients();
	float y0 = 
		coefficients[filterCoefficients::d0] * xn +
		coefficients[filterCoefficients::c0] * (
			coefficients[filterCoefficients::a0] * xn +
			coefficients[filterCoefficients::a1] * filterSamples[sampleEnum::x1] +
			coefficients[filterCoefficients::a2] * filterSamples[sampleEnum::x2] -
			coefficients[filterCoefficients::b1] * filterSamples[sampleEnum::y1] -
			coefficients[filterCoefficients::b2] * filterSamples[sampleEnum::y2]
		);

	filterSamples[sampleEnum::x2] = filterSamples[sampleEnum::x1];
	filterSamples[sampleEnum::x1] = xn;
	
	filterSamples[sampleEnum::y2] = filterSamples[sampleEnum::y1];
	filterSamples[sampleEnum::y1] = y0;

	return y0;
}

void SecondOrderLpfHpf::resized()
{
	area = getLocalBounds();
	Rectangle<int> firstKnobArea = area.removeFromLeft(area.getWidth() / 8);
	Rectangle<int> secondKnobArea = firstKnobArea.removeFromBottom(firstKnobArea.getHeight() / 2);
	
	QSlider.setBounds(firstKnobArea.reduced(10));
	QSlider.setTextBoxStyle(
		Slider::TextBoxBelow, false, firstKnobArea.reduced(10).getWidth(), QSlider.getTextBoxHeight());

	cutoffFreqSlider.setBounds(secondKnobArea.reduced(10));
	cutoffFreqSlider.setTextBoxStyle(
		Slider::TextBoxBelow, false, secondKnobArea.reduced(10).getWidth(), cutoffFreqSlider.getTextBoxHeight());
}

void SecondOrderLpfHpf::paint(Graphics &g)
{
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));   // clear the background
	g.setColour(Colours::grey);
	g.drawRect(getLocalBounds(), 1);   // draw an outline around the component
	g.setColour(Colours::white);
	g.drawRect(area, 1);

	/*for (int i = 1; i < FILTER_RESPONSE_RESOLUTION; i++)
	{
		freqResponse[i] = getMagResponse(
			i * MathConstants<float>::pi / (float)FILTER_RESPONSE_RESOLUTION,
			coefficients[filterCoefficients::a0],
			coefficients[filterCoefficients::a1],
			coefficients[filterCoefficients::a2],
			coefficients[filterCoefficients::b1],
			coefficients[filterCoefficients::b2]);
	}

	for (int i = 1; i < FILTER_RESPONSE_RESOLUTION; i++)
	{
		float prevSample = 0.0f;
		// Want ticks on scale to be 10, 100, 1000, 10000 so divide width by 4
		float scaleX = getWidth() / 4;
		// Want dB ticks from -60 to +12 in 12dB intervals
		float scaleY = getHeight() / 7;

		Path path;
		path.startNewSubPath(0, area.getHeight() / 2.0f);

		for (int i = 1; i < FILTER_RESPONSE_RESOLUTION; i++)
		{
			path.lineTo(log10(i * sampleRate /(float)(2.0f * FILTER_RESPONSE_RESOLUTION)) * scaleX, area.getHeight() - freqResponse[i] * scaleY);
		}

		g.strokePath(path, PathStrokeType(1.3f));
	}*/

}

// theta from 0 to pi
// used on audio thread
double SecondOrderLpfHpf::getMagResponse(double theta, double a0, double a1, double a2, double b1, double b2)
{
	double magSqr = 0.0;
	double num = a1 * a1 + (a0 - a2)*(a0 - a2) + 2.0*a1*(a0 + a2)*cos(theta) + 4.0*a0*a2*cos(theta)*cos(theta);
	double denom = b1 * b1 + (1.0 - b2)*(1.0 - b2) + 2.0*b1*(1.0 + b2)*cos(theta) + 4.0*b2*cos(theta)*cos(theta);

	magSqr = num / denom;
	if (magSqr < 0.0)
		magSqr = 0.0;

	double mag = pow(magSqr, 0.5);
	return mag;
}