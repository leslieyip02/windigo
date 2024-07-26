/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PitchShifter.hpp"
#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "WaveFile.hpp"

//==============================================================================
SamplerAudioProcessor::SamplerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ), ActionBroadcaster()

#endif
{

    mFormatManager.registerBasicFormats();

    for (int i = 0; i < mNumVoices; i++)
    {
        mSampler.addVoice(new juce::SamplerVoice());
    }
    stateDisplayText.setValue("Please load a file first");
}

SamplerAudioProcessor::~SamplerAudioProcessor()
{
    mFormatReader = nullptr;
    remove("./temp.wav");
}


//==============================================================================
//to allow for access in pluginEditor
juce::MidiKeyboardState& SamplerAudioProcessor::getKState()
{
    return kState;
}

const juce::String SamplerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SamplerAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool SamplerAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool SamplerAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double SamplerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SamplerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int SamplerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SamplerAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String SamplerAudioProcessor::getProgramName(int index)
{
    return {};
}

void SamplerAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void SamplerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    mSampler.setCurrentPlaybackSampleRate(sampleRate);
}

void SamplerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SamplerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
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

void SamplerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    kState.processNextMidiBuffer(midiMessages, 0, buffer.getNumSamples(), true); //to allow our keyboard to send midimessages to be received by midibuffer
    mSampler.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool SamplerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SamplerAudioProcessor::createEditor()
{
    return new SamplerAudioProcessorEditor(*this);
}

//==============================================================================
void SamplerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SamplerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void SamplerAudioProcessor::loadFile()
{
    chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
        juce::File{},
        "*.wav");                     // [7]


    chooser->launchAsync(1, [this](const juce::FileChooser& fc)     // [8]
        {
            auto file = fc.getResult();

            if (file != juce::File{})                                                // [9]
            {
                mFormatReader = mFormatManager.createReaderFor(file);                 // [10]
                if (mFormatReader != nullptr)
                {
                    mSampler.addSound(new juce::SamplerSound("Sample", *mFormatReader, juce::BigInteger().setRange(0, 128, true), 72, 0.1, 0.1, 10.0));
                    audioClip = file; //assigning the file to processor
                    currentPitch = 0;
                    sendActionMessage("Keyboard is ready");
                }
            }
        });

}

void SamplerAudioProcessor::upKey() {
    std::thread work([&]
        {
            sendActionMessage("Modulation in progress...");
            currentPitch++;
            WaveFile toBeShifted = WaveFile(audioClip.getFullPathName().toStdString());
            shifter.shift(toBeShifted, currentPitch);
            remove("./temp.wav");
            toBeShifted.write("./temp.wav");
            juce::File shifted = juce::File("./temp.wav");
            mFormatReader = mFormatManager.createReaderFor(shifted);
            mSampler.addSound(new juce::SamplerSound("Sample", *mFormatReader, juce::BigInteger().setRange(0, 128, true), 72, 0.1, 0.1, 10.0));
            sendActionMessage("Keyboard is ready");
        });
    work.detach();
}

void SamplerAudioProcessor::downKey() {
    std::thread work([&]
        {
            sendActionMessage("Modulation in progress...");
            currentPitch--;
            WaveFile toBeShifted = WaveFile(audioClip.getFullPathName().toStdString());
            shifter.shift(toBeShifted, currentPitch); //alot of distortion when downkey
            remove("./temp.wav");
            toBeShifted.write("./temp.wav");
            juce::File shifted = juce::File("./temp.wav");
            mFormatReader = mFormatManager.createReaderFor(shifted);
            mSampler.addSound(new juce::SamplerSound("Sample", *mFormatReader, juce::BigInteger().setRange(0, 128, true), 72, 0.1, 0.1, 10.0));
            sendActionMessage("Keyboard is ready");
        });
    work.detach();
}

void SamplerAudioProcessor::addOriginalSound() {
    std::thread work([&] {
        currentPitch--;
        mFormatReader = mFormatManager.createReaderFor(audioClip);
        mSampler.addSound(new juce::SamplerSound("Sample", *mFormatReader, juce::BigInteger().setRange(0, 128, true), 72, 0.1, 0.1, 10.0));
        sendActionMessage("Keyboard is ready");
        });
    work.detach();
}

int SamplerAudioProcessor::getKey()
{
    return currentPitch;
}

bool SamplerAudioProcessor::isFileLoaded()
{
    return mFormatReader != nullptr;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SamplerAudioProcessor;
}
