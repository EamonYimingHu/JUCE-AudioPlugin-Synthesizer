/*
  ==============================================================================

    Filter.h
    Created: 26 Apr 2024 6:55:18pm
    Author:  Yiming HU

  ==============================================================================
*/

#pragma once

#ifndef FILTER_MOD_H
#define FILTER_MOD_H

#include <JuceHeader.h> // for defining juce classes variables
//#include "Parameters.h" // for accessing parameters set by the user interface

/// Filter class.
/// Filter type can be set by using setType() class method.
class Filter
{
public:
    /// constructor that resets filter instance and initialises frequency range for cutoff and resonance
    /// @param juce::NormalisableRange<float>, range for cutoff frequency
    /// @param juce::NormalisableRange<float>, range for resonance


    /// process input sample
    /// @param float, input sample
    /// @return float, filter output
    float process(float _inSample, int _filterType)
    {

        // calculate frequency when using a LFO on filter's cutoffFreq
        float freq = cutoffbase + frequencyOffset;
        float res = Q;


        // Real-time calculation of the cut-off frequency and updating it on the filter
        (*this).setFrequency(freq);
        (*this).makeFilter(_filterType);

        // reset modulations
        resetModulations();
        return filter.processSingleSampleRaw(_inSample);
    }

    /// set sample rate
    /// @param float, sample rate
    void setSampleRate(float _sampleRate)
    {
        jassert(_sampleRate > 0.0f); // check sample rate value
        sampleRate = _sampleRate;
    }

    void setCutoffBase(float _frequency)
    {
        cutoffbase = _frequency;
    }

    /// set filter frequency
    /// @param float, frequency
    void setFrequency(float _frequency)
    {
        cutoff = _frequency;
    }

    /// set filter resonance
    /// @param float, frequency
    void setResonance(float _resonance)
    {
        Q = _resonance;
    }

    /// set filter type
    /// @param int, filter type (0 - low pass, 1 - high pass, 2 - band pass, 3 - notch
    void makeFilter(int _filterType)
    {
        switch (_filterType)
        {
        case 0:
            //setFilterCoefficientsFunction(&(juce::IIRCoefficients::makeLowPass));
            filter.setCoefficients(juce::IIRCoefficients::makeLowPass(sampleRate, cutoff, Q));
            //filter.setCoefficients(juce::IIRCoefficients::makeLowPass);
            break;
        case 1:
            //setFilterCoefficientsFunction(&(juce::IIRCoefficients::makeHighPass));
            filter.setCoefficients(juce::IIRCoefficients::makeHighPass(sampleRate, cutoff, Q));
            break;
        case 2:
            //setFilterCoefficientsFunction(&(juce::IIRCoefficients::makeBandPass));
            filter.setCoefficients(juce::IIRCoefficients::makeBandPass(sampleRate, cutoff, Q));
            break;
            //case 3:
            //    //setFilterCoefficientsFunction(&(juce::IIRCoefficients::makeNotchFilter));
            //    filter.setCoefficients(juce::IIRCoefficients::makeBandPass(sampleRate, cutoff, Q));
            //    break;
        }
    }


    /// set frequency offset when using a LFO on filter's cutoffFreq
/// @param float, amount convert from Amount% in UI
    void setFrequencyOffset(float _frequencyOffset)
    {
        frequencyOffset += _frequencyOffset ;
    }

    void resetModulations()
    {
        frequencyOffset = 0.0f;
    }


    
    void startNote(float _sampleRate, float _frequency, float _resonance, int _filterType)
    {
        filter.reset();
        // env.reset();

        (*this).setSampleRate(_sampleRate);
        (*this).setFrequency(_frequency);
        (*this).setCutoffBase(_frequency);
        (*this).setResonance(_resonance);
        (*this).makeFilter(_filterType);

    }

private:
    float sampleRate = 0.0f;                                                                         // sample rate [Hz]
    // base members
    juce::IIRFilter filter;                                                                          // filter instance
    juce::IIRCoefficients(*makeFilterCoefficients) (double sampleRate, double frequency, double Q); // pointer to a function with calculates filter coefficiens using specified sample rate, cutoff frequency and resonance
    // juce::ADSR env;                                                                                  // filter cutoff envelope
     // filter parameters
    float  cutoff, cutoffbase;//frequency;
    float Q;  // resonance;
    float frequencyOffset = 0.0f;

};

#endif // FILTER_MOD_H
