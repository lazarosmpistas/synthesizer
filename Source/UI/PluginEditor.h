#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class SynthesizerAudioProcessor;

class SynthesizerAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit SynthesizerAudioProcessorEditor(SynthesizerAudioProcessor& processor);
    void paint(juce::Graphics& graphics) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthesizerAudioProcessorEditor)
};
