/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class BeamformingSpeechEnhancerAudioProcessorEditor  : public AudioProcessorEditor, Slider::Listener
{
public:
    BeamformingSpeechEnhancerAudioProcessorEditor (BeamformingSpeechEnhancerAudioProcessor&);
    ~BeamformingSpeechEnhancerAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
	void sliderValueChanged(Slider * slider) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    BeamformingSpeechEnhancerAudioProcessor& processor;
	Array<String> audioFiles;
	Slider angleSelector;
	Label angleSelectorLabel;
	TextEditor textBox;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BeamformingSpeechEnhancerAudioProcessorEditor)
};
