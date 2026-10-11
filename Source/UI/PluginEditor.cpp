#include "PluginEditor.h"
#include "Plugin/PluginProcessor.h"

SynthesizerAudioProcessorEditor::SynthesizerAudioProcessorEditor(SynthesizerAudioProcessor& processor)
    : AudioProcessorEditor(processor)
{
    setOpaque(true);
    setWantsKeyboardFocus(false);
    setSize(560, 320);
}

void SynthesizerAudioProcessorEditor::paint(juce::Graphics& graphics)
{
    const auto bounds = getLocalBounds();
    graphics.fillAll(juce::Colour(0xff151a22));

    graphics.setColour(juce::Colour(0xff75ddbb));
    graphics.fillRoundedRectangle(32.0f, 34.0f, 4.0f, 42.0f, 2.0f);
    graphics.setColour(juce::Colour(0xfff0f3f8));
    graphics.setFont(juce::FontOptions(30.0f, juce::Font::bold));
    graphics.drawText(JucePlugin_Name, 48, 30, bounds.getWidth() - 80, 48, juce::Justification::centredLeft);

    graphics.setColour(juce::Colour(0xffa4afbf));
    graphics.setFont(juce::FontOptions(15.0f));
    graphics.drawText(JucePlugin_Manufacturer "  /  VST3 instrument  /  v" JucePlugin_VersionString,
                      48, 80, bounds.getWidth() - 80, 26, juce::Justification::centredLeft);

    graphics.setColour(juce::Colour(0xff202936));
    graphics.fillRoundedRectangle(32.0f, 132.0f, static_cast<float>(bounds.getWidth() - 64), 112.0f, 12.0f);
    graphics.setColour(juce::Colour(0xff75ddbb));
    graphics.setFont(juce::FontOptions(19.0f, juce::Font::bold));
    graphics.drawText("Your synth starts here", 52, 148, bounds.getWidth() - 104, 30,
                      juce::Justification::centredLeft);
    graphics.setColour(juce::Colour(0xffc3cbd7));
    graphics.setFont(juce::FontOptions(15.0f));
    graphics.drawText("This build is silent. The sound engine comes next.",
                      52, 190, bounds.getWidth() - 104, 26, juce::Justification::centredLeft);

    graphics.setColour(juce::Colour(0xffa4afbf));
    graphics.setFont(juce::FontOptions(13.0f));
    graphics.drawText("MIDI input  /  Stereo output", 32, bounds.getHeight() - 52,
                      bounds.getWidth() - 64, 24, juce::Justification::centredLeft);
}
