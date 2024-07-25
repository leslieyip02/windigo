/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PitchShifter.hpp"
#include "WaveFile.hpp"

//==============================================================================
/**
*/
class SamplerAudioProcessor :
  public juce::AudioProcessor,
  public juce::ActionBroadcaster
{
public:
  //==============================================================================
  SamplerAudioProcessor();
  ~SamplerAudioProcessor() override;

  //==============================================================================
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String& newName) override;

  //==============================================================================
  void getStateInformation(juce::MemoryBlock& destData) override;
  void setStateInformation(const void* data, int sizeInBytes) override;

  //button functions
  void loadFile();
  void upKey();
  void downKey();
  void addOriginalSound();
  int getKey();
  bool isFileLoaded();

  //keyboard and display components
  juce::MidiKeyboardState& getKState();
  juce::Value stateDisplayText;
  juce::File audioClip;


private:
  juce::Synthesiser mSampler;
  const int mNumVoices = 3;
  juce::AudioFormatManager mFormatManager;
  juce::AudioFormatReader* mFormatReader = nullptr;
  std::unique_ptr<juce::FileChooser> chooser;
  juce::MidiKeyboardState kState;
  int currentPitch = 0;
  PitchShifter shifter = PitchShifter(4096, 4);

  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerAudioProcessor)
};
