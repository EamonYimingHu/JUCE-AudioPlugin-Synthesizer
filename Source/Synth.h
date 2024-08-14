/*
  ==============================================================================

    Synth.h
    Created: 27 Mar 2024 12:50:39pm
    Author:  Yiming HU

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Oscillators.h"
#include "OscSwitch.h"
#include "Filter.h"
#include "LFO.h"

class synthSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote(int midiNoteNumber) override

    {
        return true;
    }
    bool appliesToChannel(int midiChannel) override
    {
        return true;
    }

    /** The class is reference-counted, so this is a handy pointer class for it. */
};

class synthVoice : public juce::SynthesiserVoice
{
public:

    void setParametersFromApvts(juce::AudioProcessorValueTreeState& apvts) 
    {   //AMP ADSR PARAMETER
        attackParam[0] = apvts.getRawParameterValue("attack1");
        decayParam[0] = apvts.getRawParameterValue("decay1");
        sustainParam[0] = apvts.getRawParameterValue("sustain1");
        releaseParam[0] = apvts.getRawParameterValue("release1");

        attackParam[1] = apvts.getRawParameterValue("attack2");
        decayParam[1] = apvts.getRawParameterValue("decay2");
        sustainParam[1] = apvts.getRawParameterValue("sustain2");
        releaseParam[1] = apvts.getRawParameterValue("release2");


        //OSC PARAMETER
        OscWaveshapeParam[0] = apvts.getRawParameterValue("Osc1Waveshape");
        OscWaveshapeParam[1] = apvts.getRawParameterValue("Osc2Waveshape");

        UnisonParam[0] = apvts.getRawParameterValue("Osc1Unison");
        UnisonParam[1] = apvts.getRawParameterValue("Osc2Unison");
        DetuneParam[0] = apvts.getRawParameterValue("Osc1Detune");
        DetuneParam[1] = apvts.getRawParameterValue("Osc2Detune");

        Level[0] = apvts.getRawParameterValue("level1");
        Level[1] = apvts.getRawParameterValue("level2");
        

        //filter
        filterOn = apvts.getRawParameterValue("FilterOn");
        filterType = apvts.getRawParameterValue("filterType");
        cutoffParam = apvts.getRawParameterValue("cutOff");
        QParam = apvts.getRawParameterValue("Q");

        //LFO
        lfoDestinationParam[0] = apvts.getRawParameterValue("LFO1Destination");
        lfoWaveshapeParam[0] = apvts.getRawParameterValue("LFO1Waveshape");
        lfoFreqParam[0] = apvts.getRawParameterValue("LFO1FreqParam");
        lfoAmountParam[0] = apvts.getRawParameterValue("LFO1AmountParam");

        lfoDestinationParam[1] = apvts.getRawParameterValue("LFO2Destination");
        lfoWaveshapeParam[1] = apvts.getRawParameterValue("LFO2Waveshape");
        lfoFreqParam[1] = apvts.getRawParameterValue("LFO2FreqParam");
        lfoAmountParam[1] = apvts.getRawParameterValue("LFO2AmountParam");

    }

    void startNote(int midiNoteNumber,
        float velocity,
        juce::SynthesiserSound* sound,
        int currentPitchWheelPosition) override 
    {
        playing = true;
        float freq = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);

        // Osc setting prepare
        Osc1.startNote(getSampleRate(), *OscWaveshapeParam[0], freq);
        Osc2.startNote(getSampleRate(), *OscWaveshapeParam[1], freq);

        // DetuneParam get the percentage(0-100%), here *0.1 convert it to 0-10 Hz detune amount
        // e.g. We got fundamental freq base on midinote, then +10Hz +20Hz +30Hz +40Hz(if user selected 4 unison and 100% Detune) 
        for (int i = 1; i < *UnisonParam[0] +1; i++)
        {
            Uni1[i].startNote(getSampleRate(), *OscWaveshapeParam[0], freq + 0.1* (*DetuneParam[0]) * i);
        }

        
        for (int i = 1; i < *UnisonParam[1] + 1; i++)
        {
            Uni2[i].startNote(getSampleRate(), *OscWaveshapeParam[1], freq + 0.1 * (*DetuneParam[1]) * i);
        }


        env1.setSampleRate(getSampleRate());
        env2.setSampleRate(getSampleRate());


        juce::ADSR::Parameters envPara1, envPara2;
        envPara1.attack = *attackParam[0];
        envPara1.decay = *decayParam[0];
        envPara1.sustain = *sustainParam[0];
        envPara1.release = *releaseParam[0];

        envPara2.attack = *attackParam[1];
        envPara2.decay = *decayParam[1];
        envPara2.sustain = *sustainParam[1];
        envPara2.release = *releaseParam[1];

        env1.setParameters(envPara1);
        env1.noteOn();

        env2.setParameters(envPara2);
        env2.noteOn();
        //filter setting prepare
        filter.startNote(getSampleRate(), *cutoffParam, *QParam, *filterType);

        //LFO setting prepare
        lfo1.startNote(getSampleRate(), *lfoWaveshapeParam[0], *lfoFreqParam[0], *lfoAmountParam[0]);
        lfo2.startNote(getSampleRate(), *lfoWaveshapeParam[1], *lfoFreqParam[1], *lfoAmountParam[1]);
        
        
  
    }



    void stopNote(float velocity, bool allowTailOff) override
    {
        DBG("NOTE STOPPED");
        env1.noteOff();
        env2.noteOff();

     }

    void renderNextBlock(juce::AudioBuffer<float>& outputBuffer,
        int startSample,
        int numSamples) override
    {
        if (playing)
        {
            // DSP LOOP 

            //for each sample
            for (int i = startSample; i < startSample+numSamples; i++)
            {

                // Process Unison
                //Reset UnisonSample 
                UniSample1 = 0;
                UniSample2 = 0;


                for (int i = 1; i < *UnisonParam[0] + 1; i++)
                {
                    UniSample1 +=  Uni1[i].process();
                }


                for (int i = 1; i < *UnisonParam[1] + 1; i++)
                {
                    UniSample2 += Uni2[i].process();
                }
                
                
                //Apply LFO
                //get LFO sample
                int lfo1Destination = *lfoDestinationParam[0];
                int lfo2Destination = *lfoDestinationParam[1];

                float lfo1Sample = lfo1.process();
                float lfo2Sample = lfo2.process();



                // Osc's Amp Modulated by a LFO
                if (lfo1.isAppliedToOsc1AM(lfo1Destination))
                    Osc1.setAmplitudeOffset(lfo1Sample);
                if (lfo2.isAppliedToOsc1AM(lfo2Destination))
                    Osc1.setAmplitudeOffset(lfo2Sample);
                if (lfo1.isAppliedToOsc2AM(lfo1Destination))
                    Osc2.setAmplitudeOffset(lfo1Sample);
                if (lfo2.isAppliedToOsc2AM(lfo2Destination))
                    Osc2.setAmplitudeOffset(lfo2Sample);

                // Osc's Freq Modulated by a LFO
                // LFO Original Value:-1 to 1
                // Times Amount value(in LFO.h) gives 0 or -100 to 100 (namely lfo1Sample's value here)
                // Here Times 5, so that we can control the frequency to increase or decrease by 500 Hz.
                if (lfo1.isAppliedToOsc1FM(lfo1Destination))
                    Osc1.setFreqOffset(5* lfo1Sample);
                if (lfo2.isAppliedToOsc1FM(lfo2Destination))
                    Osc1.setFreqOffset(5* lfo2Sample);
                if (lfo1.isAppliedToOsc2FM(lfo1Destination))
                    Osc2.setFreqOffset(5* lfo1Sample);
                if (lfo2.isAppliedToOsc2FM(lfo2Destination))
                    Osc2.setFreqOffset(5* lfo2Sample);


                // Osc's Phase Modulated by a LFO          
                if (lfo1.isAppliedToOsc1PM(lfo1Destination))
                    Osc1.setAmplitudeOffset(lfo1Sample);
                if (lfo2.isAppliedToOsc1PM(lfo2Destination))
                    Osc1.setAmplitudeOffset(lfo2Sample);
                if (lfo1.isAppliedToOsc2PM(lfo1Destination))
                    Osc2.setAmplitudeOffset(lfo1Sample);
                if (lfo2.isAppliedToOsc2PM(lfo2Destination))
                    Osc2.setAmplitudeOffset(lfo2Sample);


                // Filter's cutoffFrequency modulation(by a LFO)
                // LFO Original Value:-1 to 1
                // Times Amount value(in LFO.h) gives 0 or -100 to 100 (namely lfo1Sample's value here)
                // Here Times 7, so that we can control the cut-off frequency to increase or decrease by 700 Hz.
                if (lfo1.isAppliedToFilterCutoffFreq(lfo1Destination))
                    filter.setFrequencyOffset(7* lfo1Sample);
                if (lfo2.isAppliedToFilterCutoffFreq(lfo2Destination))
                    filter.setFrequencyOffset(7* lfo2Sample);


                // Normalise the results
                float outputSample1 = (Osc1.process() + UniSample1) / (*UnisonParam[0] + 1);
                float outputSample2 = (Osc2.process() + UniSample2) / (*UnisonParam[1] + 1);


                // Level Control and output
                float Osc1level = *Level[0];
                float Osc2level = *Level[1];

                float envvalue1 = env1.getNextSample();
                float envvalue2 = env2.getNextSample();

                float outputSample = envvalue1 * Osc1level * outputSample1 + envvalue2 * Osc2level * outputSample2;

                // Process Filter
                if (*filterOn == true)
                {
                    outputSample = filter.process(outputSample, *filterType);

                }
                
   
                //for each channel
                for (int chan = 0; chan < outputBuffer.getNumChannels(); chan++)
                {
                    outputBuffer.addSample(chan, i, outputSample * 0.1);
                }


                // When both of the Osc's life cycle end, clear notes
                if (!env1.isActive() && !env2.isActive() )
                {
                    playing = false;
                    clearCurrentNote();
                }
            }
        }
    }

    bool canPlaySound(juce::SynthesiserSound*)override
    {
        return true;
     }

    void pitchWheelMoved(int newPitchWheelValue) override
    {

     }

    void controllerMoved(int controllerNumber, int newControllerValue) override
    {

     }

  




private:
    juce::Random random;
    bool playing = true;
    Filter filter;
    LFO lfo1, lfo2;
    OscSwitch Osc1, Osc2;
    OscSwitch Uni1[7],Uni2[7];                               // For Osc1's Unison Effect

    float UniSample1, UniSample2;
    juce::ADSR env1, env2;

    //parameters
    std::atomic<float>* attackParam[2];
    std::atomic<float>* decayParam[2];
    std::atomic<float>* sustainParam[2];
    std::atomic<float>* releaseParam[2];

    // Osc and Unison
    std::atomic<float>* OscWaveshapeParam[2];
    std::atomic<float>* Level[2];
    std::atomic<float>* UnisonParam[2];             // define the num of unison
    std::atomic<float>* DetuneParam[2];             // store the detune amount in percentage



    // Filter Parameters
    std::atomic<float>* filterOn;
    std::atomic<float>* filterType;
    std::atomic<float>* cutoffParam;
    std::atomic<float>* QParam;

    // LFO Parameters
    std::atomic<float>* lfoDestinationParam[2];     // LFOs destinations
    std::atomic<float>* lfoWaveshapeParam[2];       // LFOs waveshapes
    std::atomic<float>* lfoFreqParam[2];            // LFOs rate
    std::atomic<float>* lfoAmountParam[2];          // LFOs amount


};
