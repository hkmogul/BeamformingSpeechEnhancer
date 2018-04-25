/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BeamformingSpeechEnhancerAudioProcessorEditor::BeamformingSpeechEnhancerAudioProcessorEditor (BeamformingSpeechEnhancerAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
	addAndMakeVisible(fileChooser);
	fileChooser.addListener(this);
	addAndMakeVisible(angleSelector);

	// to allow it to run counterclockwise = change in angle
	angleSelector.setRange(-345, 0, 15);
	angleSelector.setSliderStyle(Slider::SliderStyle::Rotary);
	angleSelector.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
	angleSelector.setValue(0);

	Slider::RotaryParameters param;
	param.startAngleRadians = 0;
	param.endAngleRadians = 2 * 3.14159;
	param.stopAtEnd = false;
	angleSelector.setRotaryParameters(param);
	angleSelector.addListener(this);

	addAndMakeVisible(textBox);
	textBox.setText("0 degrees", false);
	textBox.setReadOnly(true);
	textBox.setCaretVisible(false);


	addAndMakeVisible(angleSelectorLabel);
	angleSelectorLabel.attachToComponent(&angleSelector, true);
	angleSelectorLabel.setText("Angle (degrees)", NotificationType::dontSendNotification);
	// trigger the root folder selection right away
	
	beamFormerFiles =
	{
		"degree0.wav",
		"degree15.wav",
		"degree30.wav",
		"degree45.wav",
		"degree60.wav",
		"degree75.wav",
		"degree90.wav",
		"degree105.wav",
		"degree120.wav",
		"degree135.wav",
		"degree150.wav",
		"degree165.wav",
		"degree180.wav",
		"degree195.wav",
		"degree210.wav",
		"degree225.wav",
		"degree240.wav",
		"degree255.wav",
		"degree270.wav",
		"degree285.wav",
		"degree300.wav",
		"degree315.wav",
		"degree330.wav",
		"degree345.wav"
	};

	mFiles =
	{
		"M_degree0.wav",
		"M_degree15.wav",
		"M_degree30.wav",
		"M_degree45.wav",
		"M_degree60.wav",
		"M_degree75.wav",
		"M_degree90.wav",
		"M_degree105.wav",
		"M_degree120.wav",
		"M_degree135.wav",
		"M_degree150.wav",
		"M_degree165.wav",
		"M_degree180.wav",
		"M_degree195.wav",
		"M_degree210.wav",
		"M_degree225.wav",
		"M_degree240.wav",
		"M_degree255.wav",
		"M_degree270.wav",
		"M_degree285.wav",
		"M_degree300.wav",
		"M_degree315.wav",
		"M_degree330.wav",
		"M_degree345.wav"
	};


	baseDir = "C:\\JUCE\\sbx\\BeamformingSpeechEnhancer\\MatlabScripts\\";
	processor.updateBeamformer(File(baseDir).getChildFile(beamFormerFiles[0]).getFullPathName());
	processor.updatePostFilter(File(baseDir).getChildFile(mFiles[0]).getFullPathName());

	// Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);	
}

BeamformingSpeechEnhancerAudioProcessorEditor::~BeamformingSpeechEnhancerAudioProcessorEditor()
{
}

//==============================================================================
void BeamformingSpeechEnhancerAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
}

void BeamformingSpeechEnhancerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	angleSelector.setBoundsRelative(0.4, 0.5, 0.3, 0.3);
	textBox.setBoundsRelative(0.4, 0.85, 0.2, 0.05);
	fileChooser.setBoundsRelative(0.1, 0.1, 0.5, 0.2);
}

void BeamformingSpeechEnhancerAudioProcessorEditor::sliderValueChanged(Slider * slider)
{
	if (slider == &angleSelector)
	{
		// get the corresponding index
		int index = -1 * angleSelector.getValue() /15;
		// set the processor
		processor.updateBeamformer(File(baseDir).getChildFile(beamFormerFiles[index]).getFullPathName());
		processor.updatePostFilter(File(baseDir).getChildFile(mFiles[index]).getFullPathName());

		// update the textbox
		String m;
		m << index * 15 << " degrees";
		textBox.setText(m, false);
	}
}

void BeamformingSpeechEnhancerAudioProcessorEditor::buttonClicked(Button * button)
{
	//if (button == &selectBaseDir)
	//{
	//	// open a filechooser to select the base directory
	//}
}

void BeamformingSpeechEnhancerAudioProcessorEditor::filenameComponentChanged(FilenameComponent * cpt)
{
	if (cpt == &fileChooser)
	{
		baseDir = fileChooser.getCurrentFileText() + "\\";
		processor.updateBeamformer(baseDir + beamFormerFiles[0]);
		DBG(baseDir);
	}
}
