/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SamplerAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::ActionListener
{
public:
  SamplerAudioProcessorEditor(SamplerAudioProcessor&);
  ~SamplerAudioProcessorEditor() override;

  //==============================================================================
  void paint(juce::Graphics&) override;
  void resized() override;
  juce::Label pluginStateDisplay;
  
  void actionListenerCallback(const juce::String& message) override;




private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  juce::TextButton mLoadButton{ "Click to Load" };
  SamplerAudioProcessor& audioProcessor;
  juce::MidiKeyboardComponent keyboard;
  juce::TextButton upKeyButton{ "+" };
  juce::TextButton downKeyButton{ "-" };
  juce::TextButton enableModulationButton{ "Enable Modulation" };
  juce::Label currentKeyDisplay;
  juce::Label modulationLabel;
  juce::Label currentStatusLabel;






  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SamplerAudioProcessorEditor)
};
