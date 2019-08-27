/*
  ==============================================================================

    2ndOrderLpfHpf.h
    Created: 18 Aug 2019 1:56:47pm
    Author:  The Big G

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#define FILTER_RESPONSE_RESOLUTION 20

class SecondOrderLpfHpf : public Component,
						  public Slider::Listener,
						  public Timer
{
public:
	SecondOrderLpfHpf();
	~SecondOrderLpfHpf();
	
	enum filterCoefficients { thetaC, d, beta, gamma, a0, a1, a2, b1, b2, c0, d0, numCoefficients };
	enum sampleEnum { x1, x2, y1, y2, numSamples };
	enum filterType { LPF = 1, HPF = -1 };
	float Q = 0.00001f, freqC = 0.0f;

	// Slider override
	void sliderValueChanged(Slider* slider) override
	{
		if (slider == &cutoffFreqSlider)
		{
			freqC = cutoffFreqSlider.getValue();
		}
		else if (slider == &QSlider) 
		{
			Q = QSlider.getValue();
		}
	}

	void setSampleRate(float _sampleRate)
	{
		if (_sampleRate > 0)
			sampleRate = _sampleRate;
		else
			sampleRate = 44100;
	}

	void timerCallback()
	{
		repaint();
	}
	
	float calculateOutput(float xn);

private:
	float coefficients[filterCoefficients::numCoefficients] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	float filterSamples[sampleEnum::numSamples] = { 0.0f, 0.0f, 0.0f, 0.0f };;
	float sampleRate;
	filterType filterType = filterType::LPF;

	// Filter functions
	void calculateCoefficients();
	double getMagResponse(double theta, double a0, double a1, double a2, double b1, double b2);

	// Component overrides
	void paint(Graphics &g) override;
	void resized() override;
	float freqResponse[FILTER_RESPONSE_RESOLUTION];

	Slider cutoffFreqSlider, QSlider;
	Label  cutoffFreqLabel, QLabel;
	Rectangle<int> area;


};

