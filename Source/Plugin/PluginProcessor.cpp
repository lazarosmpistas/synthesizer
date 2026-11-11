#include "PluginProcessor.h"
#include "UI/PluginEditor.h"

#include <charconv>
#include <cmath>

namespace
{
juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for (const auto& definition : laz::parameters::envelope)
    {
        const bool sustain = definition.maximum == 1.0f;
        juce::NormalisableRange<float> range(definition.minimum, definition.maximum, sustain ? 0.001f : 0.1f);
        range.setSkewForCentre(definition.midpoint);
        auto attributes = juce::AudioParameterFloatAttributes()
            .withLabel(definition.unit)
            .withStringFromValueFunction([sustain](float value, int)
            {
                return sustain ? juce::String(value * 100.0f, 1) : juce::String(value, 1);
            })
            .withValueFromStringFunction([sustain](const juce::String& text)
            {
                return text.getFloatValue() / (sustain ? 100.0f : 1.0f);
            });
        layout.add(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { definition.id, 1 }, definition.name, range, definition.defaultValue, attributes));
    }
    return layout;
}
}

SynthesizerAudioProcessor::SynthesizerAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, laz::parameters::stateType, createParameterLayout())
{
    for (std::size_t i = 0; i < envelopeValues.size(); ++i)
        envelopeValues[i] = parameters.getRawParameterValue(laz::parameters::envelope[i].id);
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
    // Cache atomic parameter values once per callback; no ValueTree/locks here.
    // APVTS's initial defaults can be slightly off-grid after skew conversion.
    // Honour the advertised 0.1 ms steps before rounding durations to samples.
    const auto seconds = [this](std::size_t index)
    {
        return std::round(envelopeValues[index]->load() * 10.0) / 10000.0;
    };
    engine.setEnvelopeParameters({ seconds(0), seconds(1),
                                   envelopeValues[2]->load(),
                                   seconds(3) });
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
    auto state = parameters.copyState();
    state.setProperty("schemaVersion", laz::parameters::stateVersion, nullptr);
    if (const auto xml = state.createXml())
        copyXmlToBinary(*xml, destination);
}

void SynthesizerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (sizeInBytes == 0)
    {
        // M2/M3 wrote empty state. Its sound corresponds to these defaults.
        for (const auto& definition : laz::parameters::envelope)
        {
            auto* parameter = parameters.getParameter(definition.id);
            parameter->setValueNotifyingHost(parameter->convertTo0to1(definition.defaultValue));
        }
        return;
    }
    if (data == nullptr || sizeInBytes < 0)
        return;
    const auto xml = getXmlFromBinary(data, sizeInBytes);
    if (xml == nullptr || !xml->hasTagName(laz::parameters::stateType))
        return;
    const auto state = juce::ValueTree::fromXml(*xml);
    if (state.getProperty("schemaVersion").toString() != juce::String(laz::parameters::stateVersion)
        || state.getNumChildren() != static_cast<int>(laz::parameters::envelope.size()))
        return;
    // Reject incomplete or invalid settings as a whole before replacing state.
    for (const auto& definition : laz::parameters::envelope)
    {
        const auto child = state.getChildWithProperty("id", definition.id);
        if (!child.hasType("PARAM") || !child.hasProperty("value"))
            return;
        const auto valueText = child.getProperty("value").toString();
        const auto* begin = valueText.toRawUTF8();
        const auto* end = begin + valueText.getNumBytesAsUTF8();
        double value = 0.0;
        const auto parsed = std::from_chars(begin, end, value);
        if (parsed.ec != std::errc() || parsed.ptr != end || !std::isfinite(value)
            || value < definition.minimum || value > definition.maximum)
            return;
    }
    parameters.replaceState(state);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthesizerAudioProcessor();
}
