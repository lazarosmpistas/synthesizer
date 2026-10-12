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

void SynthesizerAudioProcessor::prepareToPlay(double sampleRate, int)
{
    engine.prepare(sampleRate);
}

void SynthesizerAudioProcessor::releaseResources()
{
    engine.reset();
}

void SynthesizerAudioProcessor::reset()
{
    engine.reset();
}

bool SynthesizerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    return layouts.inputBuses.isEmpty()
        && layouts.outputBuses.size() == 1
        && layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void SynthesizerAudioProcessor::processBlock(juce::AudioBuffer<float>& audio, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    // A host may provide uninitialised output memory, including extra channels.
    audio.clear();
    if (audio.getNumChannels() < 2)
    {
        midi.clear();
        return;
    }

    auto* left = audio.getWritePointer(0);
    auto* right = audio.getWritePointer(1);
    const int size = audio.getNumSamples();
    int rendered = 0;
    for (const auto metadata : midi)
    {
        const int position = juce::jlimit(rendered, size, metadata.samplePosition);
        if (position > rendered)
            engine.render(left + rendered, right + rendered, position - rendered);
        engine.handleMidi(metadata.data, metadata.numBytes);
        rendered = position;
    }
    if (size > rendered)
        engine.render(left + rendered, right + rendered, size - rendered);
    midi.clear();
}

juce::AudioProcessorEditor* SynthesizerAudioProcessor::createEditor()
{
    return new SynthesizerAudioProcessorEditor(*this);
}

void SynthesizerAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    // The fixed sound has no settings. Live voices are never saved as held notes.
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
