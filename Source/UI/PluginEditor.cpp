#include "PluginEditor.h"
#include "Plugin/PluginProcessor.h"

void SynthesizerAudioProcessorEditor::EnvelopeLookAndFeel::drawRotarySlider(
    juce::Graphics& graphics, int x, int y, int width, int height,
    float position, float startAngle, float endAngle, juce::Slider&)
{
    const auto dial = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(10.0f);
    const auto centre = dial.getCentre();
    const float radius = juce::jmin(dial.getWidth(), dial.getHeight()) * 0.5f;
    const float angle = startAngle + position * (endAngle - startAngle);
    juce::Path track;
    track.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, startAngle, endAngle, true);
    graphics.setColour(juce::Colour(0xff374354));
    graphics.strokePath(track, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    juce::Path value;
    value.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, startAngle, angle, true);
    graphics.setColour(juce::Colour(0xff75ddbb));
    graphics.strokePath(value, juce::PathStrokeType(4.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));
    graphics.setColour(juce::Colour(0xff151c26));
    graphics.fillEllipse(centre.x - radius + 8.0f, centre.y - radius + 8.0f,
                         (radius - 8.0f) * 2.0f, (radius - 8.0f) * 2.0f);
    juce::Path pointer;
    pointer.addRoundedRectangle(-2.0f, -radius + 14.0f, 4.0f, radius * 0.35f, 2.0f);
    graphics.setColour(juce::Colour(0xffedf5f3));
    graphics.fillPath(pointer, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
}

SynthesizerAudioProcessorEditor::SynthesizerAudioProcessorEditor(SynthesizerAudioProcessor& processor)
    : AudioProcessorEditor(processor)
{
    setOpaque(true);
    setWantsKeyboardFocus(false);
    for (std::size_t i = 0; i < knobs.size(); ++i)
    {
        const auto& definition = laz::parameters::envelope[i];
        auto& knob = knobs[i];
        knob.setLookAndFeel(&knobLook);
        knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        knob.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 104, 26);
        knob.setTextValueSuffix(juce::String(" ") + definition.unit);
        knob.setName(definition.name);
        knob.setComponentID(definition.id);
        knob.setTitle(juce::String(definition.name) + " envelope");
        knob.setWantsKeyboardFocus(false);
        knob.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffedf5f3));
        knob.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff151c26));
        knob.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff374354));
        addAndMakeVisible(knob);
        labels[i].setText(definition.name, juce::dontSendNotification);
        labels[i].setFont(juce::FontOptions(15.0f, juce::Font::bold));
        labels[i].setColour(juce::Label::textColourId, juce::Colour(0xffc3cbd7));
        labels[i].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(labels[i]);
        attachments[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            processor.getParameterState(), definition.id, knob);
        knob.setDoubleClickReturnValue(true, definition.defaultValue);
    }
    setSize(640, 400);
}

SynthesizerAudioProcessorEditor::~SynthesizerAudioProcessorEditor()
{
    for (auto& knob : knobs)
        knob.setLookAndFeel(nullptr);
}

void SynthesizerAudioProcessorEditor::resized()
{
    for (std::size_t i = 0; i < knobs.size(); ++i)
    {
        const int x = 40 + static_cast<int>(i) * 140;
        labels[i].setBounds(x, 190, 140, 24);
        knobs[i].setBounds(x + 8, 220, 124, 120);
    }
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
    graphics.fillRoundedRectangle(32.0f, 132.0f, static_cast<float>(bounds.getWidth() - 64), 220.0f, 12.0f);
    graphics.setColour(juce::Colour(0xff75ddbb));
    graphics.setFont(juce::FontOptions(19.0f, juce::Font::bold));
    graphics.drawText("Amplitude envelope", 52, 148, bounds.getWidth() - 104, 30,
                      juce::Justification::centredLeft);

    graphics.setColour(juce::Colour(0xffa4afbf));
    graphics.setFont(juce::FontOptions(13.0f));
    graphics.drawText("Saw  /  8 voices  /  Double-click a knob to reset", 32, bounds.getHeight() - 40,
                      bounds.getWidth() - 64, 24, juce::Justification::centredLeft);
}
