/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PolyphonicSynthAudioProcessor::PolyphonicSynthAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ),
#endif

apvts(*this, nullptr, "ParameterTree", createParameterLayout())



{
    reverbon = apvts.getRawParameterValue("Reverb");

    synth.addSound(new synthSound());

    for (int i = 0; i < voicecount; i++)
        synth.addVoice(new synthVoice());

    for (int i = 0; i < synth.getNumVoices(); i++)
    {
        auto voice = dynamic_cast  <synthVoice*>(synth.getVoice(i));
        voice->setParametersFromApvts(apvts);
    }
}

PolyphonicSynthAudioProcessor::~PolyphonicSynthAudioProcessor()
{
}

//==============================================================================
const juce::String PolyphonicSynthAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PolyphonicSynthAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PolyphonicSynthAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PolyphonicSynthAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PolyphonicSynthAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PolyphonicSynthAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PolyphonicSynthAudioProcessor::getCurrentProgram()
{
    return 0;
}

void PolyphonicSynthAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PolyphonicSynthAudioProcessor::getProgramName (int index)
{
    return {};
}

void PolyphonicSynthAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PolyphonicSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate(sampleRate);

    juce::Reverb::Parameters reverbParams;
    reverbParams.dryLevel = 0.5f;
    reverbParams.wetLevel = 0.5f;
    reverbParams.roomSize = 0.5f;

    reverb.setParameters(reverbParams);
    reverb.reset();
}

void PolyphonicSynthAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PolyphonicSynthAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PolyphonicSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // For reverb and delay
    int numSamples = buffer.getNumSamples();
    // pointers to audio arrays
    float* leftChannel = buffer.getWritePointer(0); //int channelNumber
    float* rightChannel = buffer.getWritePointer(1);

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    if (*reverbon == true)
    {
        reverb.processStereo(leftChannel, rightChannel, numSamples);
    }


}

//==============================================================================
bool PolyphonicSynthAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PolyphonicSynthAudioProcessor::createEditor()
{
    //return new PolyphonicSynthAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void PolyphonicSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void PolyphonicSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
    {
        if (xmlState->hasTagName(apvts.state.getType()))
        {
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PolyphonicSynthAudioProcessor();
}
