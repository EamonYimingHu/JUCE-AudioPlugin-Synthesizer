#ifndef LFO_H
#define LFO_H

#include <cmath>                // for math operations
#include <variant>              // for std::variant
#include "Oscillators.h"        // Assuming this file includes the oscillator definitions
#include <juce_audio_basics/juce_audio_basics.h>  // For juce::SmoothedValue

class LFO
{
public:
    // Define a variant to hold any type of oscillator
    using OscVariant = std::variant<SinOsc, TriOsc, SawOsc, SqrOsc>;

    LFO() : lfo(SinOsc{}) {} // Initialize with a default SinOsc

    float process()
    {
        // FM
        float freq = frequency + frequencyOffset;
        std::visit([freq](auto& os) { os.setFrequency(freq); }, lfo);

        // PM
        //float phase = 
        float lfoSample = (std::visit([](auto& os) { return os.process(); }, lfo));
        lfoSample = amount * lfoSample;
        smoothedLFOValue.setTargetValue(lfoSample);
        return smoothedLFOValue.getNextValue();
    }

    void setSampleRate(float _sampleRate)
    {
        std::visit([_sampleRate](auto& os) { os.setSampleRate(_sampleRate); }, lfo);
        sampleRate = _sampleRate;
        smoothedLFOValue.reset(_sampleRate, 0.01); // Smoothing interval adjustment
    }

    void setWaveshape(int _waveshapeId)
    {
        switch (_waveshapeId)
        {
        case 0:
            lfo.emplace<SinOsc>();
            break;
        case 1:
            lfo.emplace<TriOsc>();
            break;
        case 2:
            lfo.emplace<SawOsc>();
            break;
        case 3:
            lfo.emplace<SqrOsc>();
            break;
        default:
            lfo.emplace<SinOsc>(); // Default case
        }

        std::visit([this](auto& os) { os.setSampleRate(sampleRate); os.setFrequency(frequency); os.setPhase(phase); }, lfo);
        
    }

    void setFrequency(float _frequency)
    {
        frequency = _frequency;
        std::visit([_frequency](auto& os) { os.setFrequency(_frequency); }, lfo);
    }


    // This is used for one LFO's frequency modulated by another
    void setFrequencyOffset(float _frequencyOffsetAmount)
    {
        frequencyOffset += _frequencyOffsetAmount;
    }

    void setPhase(float _phase)
    {
        std::visit([_phase](auto& os) { os.setPhase(_phase); }, lfo);
    }

    void setAmount(float _amount)
    {
        amount = _amount;
    }

    void setAmountOffset(float _amountOffset)
    {
        amountOffset = _amountOffset;
    }


    void startNote(float _sampleRate, int _LFOWaveshape, float _LFOFreq, float _LFOAmount)
    {

        (*this).setSampleRate(_sampleRate);
        (*this).setWaveshape(_LFOWaveshape);
        (*this).setFrequency(_LFOFreq);
        (*this).setAmount(_LFOAmount);
        smoothedLFOValue.setCurrentAndTargetValue(0.0f);
    }
    /// check if LFO is applied to the operator level
/// @param int, LFO destination
/// @param int, number of operators in the synth
/// @return bool, true if LFO is applied
    bool isAppliedToOsc1AM(int lfoDestination)
    {
        if (lfoDestination == 0)
            return true;
        else
            return false;
    }


    bool isAppliedToOsc2AM(int lfoDestination)
    {
        if (lfoDestination == 1)
            return true;
        else
            return false;
    }

    bool isAppliedToOsc1FM(int lfoDestination)
    {
        if (lfoDestination == 2)
            return true;
        else
            return false;
    }

    bool isAppliedToOsc2FM(int lfoDestination)
    {
        if (lfoDestination == 3)
            return true;
        else
            return false;
    }

    bool isAppliedToOsc1PM(int lfoDestination)
    {
        if (lfoDestination == 4)
            return true;
        else
            return false;
    }

    bool isAppliedToOsc2PM(int lfoDestination)
    {
        if (lfoDestination == 5)
            return true;
        else
            return false;
    }


    /// check if LFO is applied to filter cutoff frequency
/// Input lfoDestination the 7th in list is FilterFreq 
/// @return bool, true if LFO is applied
    bool isAppliedToFilterCutoffFreq(int lfoDestination)
    {
        if (lfoDestination == 6)
            return true;
        else
            return false;
    }


private:
    OscVariant lfo;
    juce::SmoothedValue<float> smoothedLFOValue;
    float sampleRate = 0.0f;
    float frequency = 0.0f;
    float phase = 0.0f;
    float amount = 0.0f;
    float frequencyOffset = 0.0f;
    float amountOffset = 0.0f;
};

#endif // LFO_H
