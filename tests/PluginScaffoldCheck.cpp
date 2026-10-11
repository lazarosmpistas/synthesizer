#include "Plugin/PluginProcessor.h"

#include <array>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace
{
void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}
}

int main()
{
    juce::ScopedJuceInitialiser_GUI initialiseGui;
    try
    {
        SynthesizerAudioProcessor processor;
        require(processor.acceptsMidi() && !processor.producesMidi() && !processor.isMidiEffect(),
                "Expected a MIDI instrument without MIDI output.");
        require(processor.getTotalNumInputChannels() == 0 && processor.getTotalNumOutputChannels() == 2,
                "Expected no audio input and a stereo output.");

        auto layout = processor.getBusesLayout();
        require(processor.isBusesLayoutSupported(layout), "Stereo layout was rejected.");
        layout.outputBuses.set(0, juce::AudioChannelSet::mono());
        require(!processor.isBusesLayoutSupported(layout), "Unsupported mono layout was accepted.");
        layout.outputBuses.set(0, juce::AudioChannelSet::discreteChannels(6));
        require(!processor.isBusesLayoutSupported(layout), "Unsupported surround layout was accepted.");
        layout = processor.getBusesLayout();
        layout.inputBuses.add(juce::AudioChannelSet::stereo());
        require(!processor.isBusesLayoutSupported(layout), "Unexpected audio input was accepted.");

        // Exercise host-owned dirty output buffers, empty blocks, changing block sizes,
        // MIDI traffic and repeated prepare/release. No audio device or visible window.
        for (const auto rate : std::array<double, 3> { 44100.0, 48000.0, 96000.0 })
        {
            processor.setRateAndBufferSizeDetails(rate, 1024);
            processor.prepareToPlay(rate, 1024);
            for (const auto size : std::array<int, 5> { 0, 1, 64, 256, 1024 })
            {
                juce::AudioBuffer<float> audio(2, size);
                for (int channel = 0; channel < audio.getNumChannels(); ++channel)
                    for (int sample = 0; sample < size; ++sample)
                        audio.setSample(channel, sample, 0.75f);
                juce::MidiBuffer midi;
                if (size > 0)
                {
                    midi.addEvent(juce::MidiMessage::noteOn(1, 60, 0.8f), 0);
                    midi.addEvent(juce::MidiMessage::noteOff(1, 60), size - 1);
                }
                processor.processBlock(audio, midi);
                require(midi.isEmpty(), "Instrument emitted MIDI.");
                for (int channel = 0; channel < audio.getNumChannels(); ++channel)
                    for (int sample = 0; sample < size; ++sample)
                        require(std::isfinite(audio.getSample(channel, sample))
                                    && audio.getSample(channel, sample) == 0.0f,
                                "Scaffold emitted non-silent or invalid audio.");
            }
            processor.releaseResources();
        }

        for (int iteration = 0; iteration < 3; ++iteration)
        {
            std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
            require(editor != nullptr && editor->getWidth() == 560 && editor->getHeight() == 320,
                    "Editor could not be recreated at its expected size.");
            juce::Image preview(juce::Image::ARGB, editor->getWidth(), editor->getHeight(), true);
            juce::Graphics graphics(preview);
            editor->paint(graphics);
        }

        std::cout << "PASS: stereo MIDI instrument, silent buffers at three rates, lifecycle and editor recreation.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
