/* THIS IS THE SOURCE 
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             PlayingSoundFilesTutorial
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Plays audio files.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once
#include "../WaveFile.hpp"
#include "../PitchShifter.hpp"

//==============================================================================
class MainContentComponent   : public juce::AudioAppComponent,
                               public juce::ChangeListener
{
public:
    MainContentComponent()
        : state (Stopped)
    {
        addAndMakeVisible (&openButton);
        openButton.setButtonText ("Choose a .wav file here to be pitch shifted! Please make a copy of it first because we will overwrite the original file.");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible (&playButton);
        playButton.setButtonText ("Play");
        playButton.onClick = [this] { playButtonClicked(); };
        playButton.setColour (juce::TextButton::buttonColourId, juce::Colours::green);
        playButton.setEnabled (false);

        addAndMakeVisible (&stopButton);
        stopButton.setButtonText ("Stop");
        stopButton.onClick = [this] { stopButtonClicked(); };
        stopButton.setColour (juce::TextButton::buttonColourId, juce::Colours::red);
        stopButton.setEnabled (false);

        addAndMakeVisible(stepInput);
        stepInput.addItem("+1", 1);
        stepInput.addItem("-1", 2); 
        stepInput.setText("Pitch shift steps");
        stepInput.onChange = [this] { stepInputChanged(); };

        addAndMakeVisible(shiftButton);
        shiftButton.setButtonText("Shift (It's morbin time!)");
        shiftButton.onClick = [this]{ shiftButtonClicked(); };
        shiftButton.setEnabled(false);



        setSize (300, 200);

        formatManager.registerBasicFormats();       // [1]
        transportSource.addChangeListener (this);   // [2]

        setAudioChannels (0, 2);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        transportSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        if (readerSource.get() == nullptr)
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }

        transportSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        transportSource.releaseResources();
    }

    void resized() override
    {
        openButton.setBounds (10, 10, getWidth() / 2, 80);
        playButton.setBounds (10, 100, getWidth() / 2, 80);
        stopButton.setBounds (10, 190, getWidth() / 2, 80);
        stepInput.setBounds(getWidth() / 2 + 20, 10, 100, 50);
        shiftButton.setBounds(getWidth() / 2 + 20, 70, 100, 50);
    }

    void changeListenerCallback (juce::ChangeBroadcaster* source) override
    {
        if (source == &transportSource)
        {
            if (transportSource.isPlaying())
                changeState (Playing);
            else
                changeState (Stopped);
        }
    }

    

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void changeState (TransportState newState)
    {
        if (state != newState)
        {
            state = newState;

            switch (state)
            {
                case Stopped:                           // [3]
                    stopButton.setEnabled (false);
                    playButton.setEnabled (true);
                    transportSource.setPosition (0.0);
                    break;

                case Starting:                          // [4]
                    playButton.setEnabled (false);
                    transportSource.start();
                    break;

                case Playing:                           // [5]
                    stopButton.setEnabled (true);
                    break;

                case Stopping:                          // [6]
                    transportSource.stop();
                    break;
            }
        }
    }

    void openButtonClicked()
    {
        chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
            juce::File{},
            "*.wav");                     // initialising choose file dialog
        auto chooserFlags = juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)     //creation of choose file dialog
            {
                juceFile = fc.getResult();
                if (stepsChosen) {
                    shiftButton.setEnabled(true);
                }
            });      
    }

    void playButtonClicked()
    {
        changeState (Starting);
    }

    void stopButtonClicked()
    {
        changeState (Stopping);
    }

    void shiftButtonClicked() //conversion from juce File to WaveFile and back using fileDirectory and passing on to pitch shifter
    {
        //convert to WaveFile using directory of chosen file
        juce::String fileDirectory = juceFile.getFullPathName();
        //WaveFile takes in std::string, juce::fileDirectory is juce::String
        //juce::String is different from std::String so conversion is needed via .toStdString()
        WaveFile toBeShifted = WaveFile(fileDirectory.toStdString());

        //create PitchShifter object
        PitchShifter shifter = PitchShifter(4096, 4);

        // second argument is to be provided by userr
        shifter.shift(toBeShifted, steps);

        //we must write to a file after shifting, in this case, we wrote back to the old file,
        // effectively replacing the old whistle with the new
        toBeShifted.write(fileDirectory.toStdString());

        juce::File file = juce::File(fileDirectory); // conversion of .wav file to juce::File

        if (file != juce::File{}) {
            auto* reader = formatManager.createReaderFor(file);   // creation of format reader for .wav file in juce::File

            if (reader != nullptr)
            {
                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);   // [11]
                transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);       // [12]
                playButton.setEnabled(true);                                                      // [13]
                readerSource.reset(newSource.release());                                          // [14]
            }
        }


    }

    void stepInputChanged()
    {
        switch (stepInput.getSelectedId())
        {
        case 1: steps = 1; 
            if (juceFile != juce::File{}) {
                shiftButton.setEnabled(true);
            }
            stepsChosen = true;
            break;

        case 2: steps = -1; 
            if (juceFile != juce::File{}) {
                shiftButton.setEnabled(true);
            }
            stepsChosen = true;
            break;

        default: break;
        }
    }


    //==========================================================================
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::ComboBox stepInput; // for user to select whether to shift up or down
    juce::TextButton shiftButton; // to start pitch shift process of writing to old file
    juce::File juceFile; //variable to hold the .wav file that is chosen by Juce Dialog
    bool stepsChosen = false;
    int steps = 0;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    TransportState state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
