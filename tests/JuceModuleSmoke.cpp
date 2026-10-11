// Dependency integration check only; this is not the product's sound engine.
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include <cmath>
#include <iostream>

int main()
{
    juce::ScopedJuceInitialiser_GUI initialiseGui;
    constexpr double sampleRate = 48000.0;

    juce::dsp::Oscillator<float> oscillator;
    oscillator.initialise([](float phase) { return std::sin(phase); });
    oscillator.prepare({ sampleRate, 256, 1 });
    oscillator.setFrequency(440.0f, true);

    juce::ADSR envelope;
    envelope.setSampleRate(sampleRate);
    envelope.setParameters({ 0.01f, 0.01f, 0.5f, 0.02f });
    envelope.noteOn();

    double energy = 0.0;
    for (int i = 0; i < 2400; ++i)
    {
        const auto sample = oscillator.processSample(0.0f) * envelope.getNextSample();
        if (!std::isfinite(sample))
            return 1;
        energy += static_cast<double>(sample) * sample;
    }
    envelope.noteOff();
    for (int i = 0; i < 1200; ++i)
        envelope.getNextSample();
    if (energy <= 0.0 || envelope.isActive() || envelope.getNextSample() != 0.0f)
    {
        std::cerr << "Oscillator/envelope module check failed.\n";
        return 1;
    }

    juce::Synthesiser voices;
    voices.setCurrentPlaybackSampleRate(sampleRate);
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 69, 0.8f), 0);
    midi.addEvent(juce::MidiMessage::noteOff(1, 69), 128);
    juce::AudioBuffer<float> buffer(2, 256);
    buffer.clear();
    voices.renderNextBlock(buffer, midi, 0, buffer.getNumSamples());

    juce::AudioParameterFloat gain(juce::ParameterID { "smokeGain", 1 }, "Gain", 0.0f, 1.0f, 0.5f);
    juce::Slider slider;
    slider.setRange(0.0, 1.0);
    slider.setValue(gain.get(), juce::dontSendNotification);
    if (std::abs(slider.getValue() - 0.5) > 0.0001)
        return 1;

    std::cout << "PASS: " << juce::SystemStats::getJUCEVersion()
              << " audio, DSP, voice/MIDI, parameter, and GUI modules linked and ran.\n";
    return 0;
}
