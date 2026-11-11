#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

class SynthesizerAudioProcessor;

class SynthesizerAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit SynthesizerAudioProcessorEditor(SynthesizerAudioProcessor& processor);
    ~SynthesizerAudioProcessorEditor() override;
    void paint(juce::Graphics& graphics) override;
    void resized() override;

private:
    class EnvelopeLookAndFeel final : public juce::LookAndFeel_V4
    {
    public:
        void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                              float position, float startAngle, float endAngle, juce::Slider&) override;
    };

    EnvelopeLookAndFeel knobLook;
    std::array<juce::Slider, 4> knobs;
    std::array<juce::Label, 4> labels;
    std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>, 4> attachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthesizerAudioProcessorEditor)
};
