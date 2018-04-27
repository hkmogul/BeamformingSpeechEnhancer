/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>
const int FFT_ORDER = 256;

//==============================================================================
BeamformingSpeechEnhancerAudioProcessor::BeamformingSpeechEnhancerAudioProcessor() 
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	postFilterReady = false;
	bfFilterReady = false;
}

BeamformingSpeechEnhancerAudioProcessor::~BeamformingSpeechEnhancerAudioProcessor()
{
}

int getOrder(int bufferSize)
{
	int size = bufferSize / 2;
	int ord = 0;
	while (pow(2, ord) < size)
	{
		ord++;
	}

	return ord - 1;
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
	beamformerFilter.prepare(spec);
	postFilter.prepare(spec);
	srate = sampleRate;
	blocksize = samplesPerBlock;
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

	if (!postFilterReady || !bfFilterReady)
	{
		buffer.clear();
		return;
	}

	// copy input to a block to handle beamforming (get Z)
	DBG("BEAMFORM STAGE");
	AudioBuffer<float> tempBlock(totalNumInputChannels, buffer.getNumSamples());
	tempBlock.copyFrom(0, 0, buffer.getReadPointer(0), buffer.getNumChannels());
	tempBlock.copyFrom(1, 0, buffer.getReadPointer(1), buffer.getNumChannels());
	dsp::AudioBlock<float> beamformingBlock(tempBlock);
	if (bfFilterReady)
	{
		dsp::ProcessContextReplacing<float> context(beamformingBlock);
		beamformerFilter.process(context);

	}

	auto *bfL = beamformingBlock.getChannelPointer(0);
	auto *bfR = beamformingBlock.getChannelPointer(1);
	// clear the array
	zKArr.clearQuick();
	zKArr.resize(buffer.getNumSamples());
	//zKArr = Array<float>(bfL, buffer.getNumSamples());
	auto *zPtr = zKArr.getRawDataPointer();

	for (int idx = 0; idx < buffer.getNumSamples(); ++idx)
	{
		zPtr[idx] += bfR[0];
	}

	// now we have z[n]. do FFTs to get Z[k], Yl[k], Yr[k]
	// FFT class will do this in place.  need to copy the input buffers to prevent them getting destroyed

	// create arrays to copy data
	DBG("COPYING INPUT DATA");

	auto *inLeft = buffer.getReadPointer(0);
	auto *inRight = buffer.getReadPointer(1);
	ylKArr.clearQuick();
	yrKArr.clearQuick();
	ylKArr.resize(buffer.getNumSamples());
	yrKArr.resize(buffer.getNumSamples());
	//ylKArr = Array<float>(inLeft, buffer.getNumSamples());
	//yrKArr = Array<float>(inRight, buffer.getNumSamples());
	auto *yLK = ylKArr.getRawDataPointer();
	auto *yRK = yrKArr.getRawDataPointer();
	memcpy(yLK, inLeft, sizeof(float) * buffer.getNumSamples());
	memcpy(yRK, inLeft, sizeof(float) * buffer.getNumSamples());

	// ZERO PAD ARRAYS THAT ARE GOING TO BE SENT TO FFT TO BE 4 * (2^ORDER), since first half of pointer is considered input
	int fftOrd = getOrder(buffer.getNumSamples());
	int newSize = 4 * pow(2, fftOrd);
	zKArr.resize(newSize);
	ylKArr.resize(newSize);
	yrKArr.resize(newSize);

	// size of the FFT is denoted by blocksize - ends up being of size 2^fftOrd
	// the first half of the samples in the in/out 
	// need to find smallest possible ord
	DBG("PERFORMING FFT");

	dsp::FFT fftOperator(fftOrd);
	fftOperator.performFrequencyOnlyForwardTransform(zPtr);
	fftOperator.performFrequencyOnlyForwardTransform(yLK);
	fftOperator.performFrequencyOnlyForwardTransform(yRK);
	giBuff.setSize(1, buffer.getNumSamples(), false, false, false);
	auto *giPtr = giBuff.getWritePointer(0);
	// now we have frequency domain component forms of the signal
	// calculate dynamic portion of superdirective / postfilter combo
	DBG("CALCULATING Gk");

	for (int i = 0; i < fftOperator.getSize(); i++)
	{
		giPtr[i] = pow(zPtr[i], 3) / (pow(yLK[i], 3) + yLK[i] * pow(yRK[i], 2) + yRK[i] * pow(yLK[i], 2) + pow(yRK[i], 3));
	}

	// use inverse transform to calculate time domain coefficients
	fftOperator.performRealOnlyInverseTransform(giPtr);

	// load coefficients to processor chain
	postFilter.get<0>().getProcessor().
		copyAndLoadImpulseResponseFromBuffer(giBuff, srate, false, false, true, giBuff.getNumSamples());

	// use postfilter chain to process the original input
	DBG("POST PROCESSING");

	dsp::AudioBlock<float> inputBuffer(buffer);
	if (postFilterReady)
	{
		DBG("ACTUALLY POST PROCESSING");
		dsp::ProcessContextReplacing<float> processContext(inputBuffer);
		postFilter.process(processContext);
	}
	else
	{
		DBG("NOT READY NOT DOING POST");
	}

	DBG("PROCESS COMPLETE");
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

void BeamformingSpeechEnhancerAudioProcessor::updateBeamformer(String filename)
{

	File f(filename);
	if (f.exists())
	{
		beamformerFilter.getProcessor().loadImpulseResponse(f, true, false, f.getSize(), false);
		bfFilterReady = true;
	}
	else
	{
		DBG("File not found");
	}
}

void BeamformingSpeechEnhancerAudioProcessor::updatePostFilter(String filename)
{
	File f(filename);
	if (f.exists())
	{
		postFilter.get<1>().loadImpulseResponse(f, false, false, f.getSize(), false);
		postFilterReady = true;
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
