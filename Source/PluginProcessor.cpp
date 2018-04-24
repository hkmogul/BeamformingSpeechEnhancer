/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
BeamformingSpeechEnhancerAudioProcessor::BeamformingSpeechEnhancerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::mono(), true)
                     #endif
                       )
#endif
{
}

BeamformingSpeechEnhancerAudioProcessor::~BeamformingSpeechEnhancerAudioProcessor()
{
}

//==============================================================================
const String BeamformingSpeechEnhancerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool BeamformingSpeechEnhancerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool BeamformingSpeechEnhancerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool BeamformingSpeechEnhancerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double BeamformingSpeechEnhancerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int BeamformingSpeechEnhancerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BeamformingSpeechEnhancerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void BeamformingSpeechEnhancerAudioProcessor::setCurrentProgram (int index)
{
}

const String BeamformingSpeechEnhancerAudioProcessor::getProgramName (int index)
{
    return {};
}

void BeamformingSpeechEnhancerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void BeamformingSpeechEnhancerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	dsp::ProcessSpec spec;
	spec.numChannels = 2;
	spec.maximumBlockSize = samplesPerBlock;
	spec.sampleRate - sampleRate;
	processor.prepare(spec);
}

void BeamformingSpeechEnhancerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool BeamformingSpeechEnhancerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
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

void BeamformingSpeechEnhancerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
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

	dsp::AudioBlock<float> block(buffer);
	dsp::ProcessContextReplacing<float> context(block);
	processor.process(context);

	// now, use the context to replace the output
	for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
	{
		// get write pointer to output
		auto * writePtr = buffer.getWritePointer(i);

		// get read ptr to inputs
		auto *lRead = buffer.getReadPointer(0);
		auto *rRead = buffer.getReadPointer(1);

		for (int j = 0; j < buffer.getNumSamples(); ++j)
		{
			writePtr[j] = lRead[j] + rRead[j];
		}
	}
    
}

//==============================================================================
bool BeamformingSpeechEnhancerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* BeamformingSpeechEnhancerAudioProcessor::createEditor()
{
    return new BeamformingSpeechEnhancerAudioProcessorEditor (*this);
}

//==============================================================================
void BeamformingSpeechEnhancerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BeamformingSpeechEnhancerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void BeamformingSpeechEnhancerAudioProcessor::updateProcessor(String filename)
{
	DBG(filename);

	File f(filename);
	if (f.exists())
	{
		processor.getProcessor().loadImpulseResponse(f, true, false, f.getSize(), false);
	}
	else
	{
		DBG("File not found");
	}
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BeamformingSpeechEnhancerAudioProcessor();
}
