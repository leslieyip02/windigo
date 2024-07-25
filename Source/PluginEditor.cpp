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
            upKeyButton.setEnabled(false);
            downKeyButton.setEnabled(false);
            currentKeyDisplay.setText("0", juce::dontSendNotification);
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
            if (audioProcessor.getKey() < 11) {
                pluginStateDisplay.setText("Modulation in progress...", juce::dontSendNotification);
                currentKeyDisplay.setText(juce::String(audioProcessor.getKey() + 1), juce::dontSendNotification);
                audioProcessor.upKey();
            }
        };

    addAndMakeVisible(downKeyButton);
    downKeyButton.setEnabled(false);
    downKeyButton.onClick = [this] {
        if (audioProcessor.getKey() > 1) {
            pluginStateDisplay.setText("Modulation in progress", juce::dontSendNotification);
            currentKeyDisplay.setText(juce::String(audioProcessor.getKey() - 1), juce::dontSendNotification);
            audioProcessor.downKey();
        }

        if (audioProcessor.getKey() == 1) {
            pluginStateDisplay.setText("Modulation in progress", juce::dontSendNotification);
            currentKeyDisplay.setText(juce::String(0), juce::dontSendNotification);
            audioProcessor.addOriginalSound();
        }
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
    pluginStateDisplay.setColour(juce::Label::outlineColourId, juce::Colours::darkgrey);
    pluginStateDisplay.setJustificationType(juce::Justification::centred);
    pluginStateDisplay.setFont(juce::Font(18.0));

    addAndMakeVisible(currentStatusLabel);
    currentStatusLabel.attachToComponent(&pluginStateDisplay, false);
    currentStatusLabel.setText("Current Status:", juce::dontSendNotification);
    currentStatusLabel.setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
    currentStatusLabel.setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);

    setSize(750, 300);
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
    mLoadButton.setBounds(getWidth() / 2 - 300, getHeight() / 2 - 84, 100, 60);
    keyboard.setBounds(getWidth() / 2 - 300, getHeight() / 2 - 15, 600, 150);
    pluginStateDisplay.setBounds(getWidth() / 2 - 190, getHeight() / 2 - 60, 300, 36);

    //modulation components
    upKeyButton.setBounds(getWidth() - 140, getHeight() / 2 - 52, 60, 27);
    downKeyButton.setBounds(getWidth() - 260, getHeight() / 2 - 52, 60, 27);
    currentKeyDisplay.setBounds(getWidth() - 200, getHeight() / 2 - 52, 60, 27);
    enableModulationButton.setBounds(getWidth() - 260, getHeight() / 2 - 79, 180, 27);
    
}

void SamplerAudioProcessorEditor::actionListenerCallback(const juce::String& message)
{
    pluginStateDisplay.setText(message, juce::dontSendNotification);
}


