/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SineWaveSynAudioProcessor::SineWaveSynAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    tree(*this, nullptr, "PARAM", createParameter())
#endif
{
    mySynth.clearSounds();
    mySynth.addSound(new SynthSound());
    
    mySynth.clearVoices();
    for (int i = 0; i < 5; i++)
    {
        mySynth.addVoice(new SynthVoice());
    }
}

SineWaveSynAudioProcessor::~SineWaveSynAudioProcessor()
{
}

//==============================================================================
const juce::String SineWaveSynAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SineWaveSynAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SineWaveSynAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SineWaveSynAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SineWaveSynAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SineWaveSynAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SineWaveSynAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SineWaveSynAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SineWaveSynAudioProcessor::getProgramName (int index)
{
    return {};
}

void SineWaveSynAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SineWaveSynAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    lastSampleRate = sampleRate;
    mySynth.setCurrentPlaybackSampleRate(sampleRate);
}

void SineWaveSynAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SineWaveSynAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SineWaveSynAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (int channel = 0; channel < mySynth.getNumVoices(); ++channel)
    {
        auto* myVoice = dynamic_cast<SynthVoice*>(mySynth.getVoice(channel));
        myVoice->setLevel(tree.getRawParameterValue("GAIN")->load());
        myVoice->setWaveType(tree.getRawParameterValue("TYPE")->load());
    }
    mySynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool SineWaveSynAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SineWaveSynAudioProcessor::createEditor()
{
    return new SineWaveSynAudioProcessorEditor (*this);
}

//==============================================================================
void SineWaveSynAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SineWaveSynAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SineWaveSynAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout SineWaveSynAudioProcessor::createParameter()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>("GAIN", "Gain", 0.0f, 1.0f, 0.1f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(
                                                                  "TYPE",
                                                                  "Type",
                                                                  juce::StringArray({"Sine", "Square", "Triangle", "Saw"}),
                                                                  0));
    
    return { params.begin(), params.end() };
}
