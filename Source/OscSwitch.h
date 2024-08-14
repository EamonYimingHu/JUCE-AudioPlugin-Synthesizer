#ifndef OSC_SWITCH_H
#define OSC_SWITCH_H

#include <cmath>         // for round()
#include <variant>       // for std::variant
#include "Oscillators.h" // for using Phasor class and its subclasses

class OscSwitch
{
public:
    // Define a variant to hold any type of oscillator
    using OscVariant = std::variant<SinOsc, TriOsc, SawOsc, SqrOsc>;

    OscSwitch() : osc(SinOsc{}) {} // Initialize with a default SinOsc

    float process()
    {
        float freq = freqbase + freqOffset;
        setFrequency(freq);
        resetModulations();
        return std::visit([](auto& os) { return os.process(); }, osc);
    }

    void setSampleRate(float _sampleRate)
    {
        jassert(_sampleRate > 0.0f);
        sampleRate = _sampleRate;
        std::visit([_sampleRate](auto& os) { os.setSampleRate(_sampleRate); }, osc);
    }

    void setWaveshape(int _waveshapeId)
    {
        switch (_waveshapeId)
        {
        case 0:
            osc.emplace<SinOsc>();
            break;
        case 1:
            osc.emplace<TriOsc>();
            break;
        case 2:
            osc.emplace<SawOsc>();
            break;
        case 3:
            osc.emplace<SqrOsc>();
            break;
        default:
            osc.emplace<SinOsc>(); // Default case
        }

        // Apply current settings to the new oscillator
        std::visit([this](auto& os) { os.setSampleRate(sampleRate); os.setFrequency(frequency); os.setPhase(phase); }, osc);
    }



    void setFrequency(float _frequency)
    {
        frequency = _frequency;
        std::visit([_frequency](auto& os) { os.setFrequency(_frequency); }, osc);
    }

    void setFreqBase(float _frequency)
    {
        freqbase = _frequency;
    }

    void setPhase(float _phase)
    {
        phase = _phase;
        std::visit([_phase](auto& os) { os.setPhase(_phase); }, osc);
    }



    void setFreqOffset(float _freqOffset)
    {
        freqOffset += _freqOffset;
        std::visit([_freqOffset](auto& os) { os.setFreqOffset(_freqOffset); }, osc);
    }

    void setPhaseOffset(float _phaseOffset)
    {
        phaseOffset += _phaseOffset;
        std::visit([_phaseOffset](auto& os) { os.setPhaseOffset(_phaseOffset); }, osc);
    }

    

    void setAmplitudeOffset(float _amplitudeOffset)
    {
        amplitudeOffset += _amplitudeOffset;
        std::visit([_amplitudeOffset](auto& os) { os.setAmplitudeOffset(_amplitudeOffset); }, osc);
    }

    

    void startNote(float _sampleRate, int _OscWaveshape, int _OscFreq)
    {
        (*this).setSampleRate(_sampleRate);
        (*this).setWaveshape(_OscWaveshape);
        (*this).setFrequency(_OscFreq);
        (*this).setFreqBase(_OscFreq);
        
    }

    void resetModulations()
    {
        amplitudeOffset = 0.0f;
        setAmplitudeOffset(0.0f);
        freqOffset = 0.0f;
        setFreqOffset(0.0f);
        phaseOffset = 0.0f;
        setPhaseOffset(0.0f);
    }

private:
    OscVariant osc; // Variant to hold any oscillator type

    float sampleRate = 0.0f;
    float frequency = 0.0f;
    float freqbase = 0.0f;
    float phase = 0.0f;
    float phasebase = 0.0f;
    float phaseOffset = 0.0f;
    float freqOffset = 0.0f;
    float amplitude = 1.0f;
    float amplitudeOffset = 0.0f;
};

#endif // OSC_SWITCH_H
