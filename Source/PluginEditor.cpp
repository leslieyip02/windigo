/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
SamplerAudioProcessorEditor::SamplerAudioProcessorEditor(SamplerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
    keyboard(audioProcessor.getKState(), juce::MidiKeyboardComponent::horizontalKeyboard), ActionListener()
    //editor must inherit constructor from juce::midiKeyboard in order to create the keyboard component
    //editor inherits from listener in order to receive change to pluginStatusDisplay in processor

{
    //keyboard components
    mLoadButton.onClick = [this]
        {
            audioProcessor.loadFile();

        };

    audioProcessor.addActionListener(this); //this is so that we can update the pluginStateDisplay when we load file into audio processor
    addAndMakeVisible(mLoadButton);
    addAndMakeVisible(keyboard);
    addAndMakeVisible(pluginStateDisplay);


    //modulation components
    addAndMakeVisible(upKeyButton);
    upKeyButton.setEnabled(false);
    upKeyButton.onClick = [this]
        {
            audioProcessor.upKey();
            currentKeyDisplay.setText(juce::String(audioProcessor.getKey()), juce::dontSendNotification);
            pluginStateDisplay.setText("Modulation in progress", juce::dontSendNotification);
        };

    addAndMakeVisible(downKeyButton);
    downKeyButton.setEnabled(false);
    downKeyButton.onClick = [this]
        {
            audioProcessor.downKey();
            currentKeyDisplay.setText(juce::String(audioProcessor.getKey()), juce::dontSendNotification);
            pluginStateDisplay.setText("Modulation in progress", juce::dontSendNotification);
        };

    addAndMakeVisible(enableModulationButton);
    enableModulationButton.onClick = [this]
        {
            upKeyButton.setEnabled(audioProcessor.isFileLoaded());
            downKeyButton.setEnabled(audioProcessor.isFileLoaded());
        };

    addAndMakeVisible(currentKeyDisplay);
    currentKeyDisplay.setColour(juce::Label::backgroundColourId, juce::Colours::white);
    currentKeyDisplay.setColour(juce::Label::textColourId, juce::Colours::black);
    currentKeyDisplay.setText("0", juce::dontSendNotification);
    currentKeyDisplay.setJustificationType(juce::Justification::centred);

    pluginStateDisplay.setText("Please load a file first", juce::dontSendNotification);
    pluginStateDisplay.setColour(juce::Label::textColourId, juce::Colours::black);
    pluginStateDisplay.setColour(juce::Label::backgroundColourId, juce::Colours::white);
    pluginStateDisplay.setColour(juce::Label::outlineColourId, juce::Colours::turquoise);
    setSize(1000, 600);
}

SamplerAudioProcessorEditor::~SamplerAudioProcessorEditor()
{
}



//==============================================================================
void SamplerAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);

}

void SamplerAudioProcessorEditor::resized()
{
    // keyboard components
    mLoadButton.setBounds(getWidth() / 2 - 200, getHeight() / 2 - 50, 100, 100);
    keyboard.setBounds(getWidth() / 2 - 300, getHeight() / 2 + 100, 600, 150);
    pluginStateDisplay.setBounds(getWidth() / 2 + 50, getHeight() / 2 - 100, 150, 30);


    //modulation components
    upKeyButton.setBounds(getWidth() - 240, 300, 60, 20);
    downKeyButton.setBounds(getWidth() - 360, 300, 60, 20);
    currentKeyDisplay.setBounds(getWidth() - 300, 300, 60, 20);
    enableModulationButton.setBounds(getWidth() - 360, 280, 180, 20);
}

void SamplerAudioProcessorEditor::actionListenerCallback(const juce::String& message)
{
    pluginStateDisplay.setText(message, juce::dontSendNotification);
}


