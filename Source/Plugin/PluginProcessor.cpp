#include "PluginProcessor.h"
#include "UI/PluginEditor.h"

SynthesizerAudioProcessor::SynthesizerAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

const juce::String SynthesizerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

void SynthesizerAudioProcessor::prepareToPlay(double, int)
{
    // No sound engine or resources yet. M3 will prepare the engine here.
}

void SynthesizerAudioProcessor::releaseResources()
{
}

bool SynthesizerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.inputBuses.isEmpty()
        && layouts.outputBuses.size() == 1
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SynthesizerAudioProcessor::processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi)
{
    // A host may provide uninitialised output memory. Always emit silence in M2.
    audio.clear();
    midi.clear();
}

juce::AudioProcessorEditor* SynthesizerAudioProcessor::createEditor()
{
    return new SynthesizerAudioProcessorEditor(*this);
}

void SynthesizerAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    // This scaffold has no parameters or user settings to persist.
    destination.reset();
}

void SynthesizerAudioProcessor::setStateInformation(const void*, int)
{
    // No state exists yet. Add versioned state when the first controls are added.
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthesizerAudioProcessor();
}
