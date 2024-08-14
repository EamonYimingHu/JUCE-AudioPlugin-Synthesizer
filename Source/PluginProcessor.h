/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "Synth.h"

//==============================================================================
/**
*/
class PolyphonicSynthAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PolyphonicSynthAudioProcessor();
    ~PolyphonicSynthAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    juce::Synthesiser synth;
    int voicecount = 4;
    juce::Reverb reverb;

    std::atomic<float>* reverbon;


    //UI
    juce::AudioProcessorValueTreeState apvts;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        juce::AudioProcessorValueTreeState::ParameterLayout layout;
        // Osc1
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("Osc1Waveshape", 1), "Osc1", juce::StringArray{ "Sine", "Triangle", "Saw", "Square"}, 0));
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("Osc1Unison", 1), "Unison", juce::StringArray{ "None", "2", "3", "4", "5", "6", "7", "8" }, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Osc1Detune", 1), "Detune(%)", 0.0, 100, 20));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("level1", 1), "Level", 0.0, 1, 0.5));

        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("attack1", 1), "Attack", 0.1, 5, 1));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("decay1", 1), "Decay", 0.0, 5, 0.5));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sustain1", 1), "Sustain", 0.0, 1, 0.4));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("release1", 1), "Release", 0.0, 5, 0.5));

        // Osc2
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("Osc2Waveshape", 1), "Osc2", juce::StringArray{ "Sine", "Triangle", "Saw", "Square"}, 0));
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("Osc2Unison", 1), "Unison", juce::StringArray{ "None", "2", "3", "4", "5", "6", "7", "8"}, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Osc2Detune", 1), "Detune(%)", 0.0, 100, 30));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("level2", 1), "Level", 0.0, 1, 0.5));

        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("attack2", 1), "Attack", 0.1, 5, 1));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("decay2", 1), "Decay", 0.0, 5, 0.5));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("sustain2", 1), "Sustain", 0.0, 1, 0.4));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("release2", 1), "Release", 0.0, 5, 0.5));


        // Reverb
        layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("Reverb", 1), "Reverb", false));

        // Filter
        layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("FilterOn", 1), "Filter On", false));
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("filterType", 1), "Filter",juce::StringArray({ "LowPass","HighPass","BandPass" }), 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("cutOff", 1), "Cutoff Freq", 100, 1000, 120));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("Q", 1), "Resonance", 0.0, 1, 0.2));

        // LFO
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("LFO1Waveshape", 1), "LFO1", juce::StringArray{ "Sine", "Triangle", "Saw", "Square" }, 0));
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("LFO1Destination", 1), "LFO1Target", juce::StringArray{ "Osc1:AM", "Osc2:AM", "Osc1:FM", "Osc2:FM", "Osc1:PM", "Osc2:PM","FilterCutoffFreq" }, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("LFO1FreqParam", 1), "LFO1Freq", 0.00, 2.00, 1.00));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("LFO1AmountParam", 1), "LFO1Amount(%)", 0.0, 100, 0.00));

        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("LFO2Waveshape", 1), "LFO2", juce::StringArray{ "Sine", "Triangle", "Saw", "Square" }, 0));
        layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID("LFO2Destination", 1), "LFO2Target", juce::StringArray{ "Osc1:AM", "Osc2:AM", "Osc1:FM", "Osc2:FM", "Osc1:PM", "Osc2:PM","FilterCutoffFreq" }, 0));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("LFO2FreqParam", 1), "LFO2Freq", 0.00, 2.00, 1.00));
        layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("LFO2AmountParam", 1), "LFO2Amount(%)", 0.0, 100, 0.00));


        return layout;
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PolyphonicSynthAudioProcessor)
};
