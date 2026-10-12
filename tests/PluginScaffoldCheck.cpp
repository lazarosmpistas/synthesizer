#include "Plugin/PluginProcessor.h"

#include <array>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <vector>

namespace
{
void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);
}

using Samples = std::vector<float>;

Samples render(SynthesizerAudioProcessor& processor, int size, juce::MidiBuffer midi = {})
{
    juce::AudioBuffer<float> audio(2, size);
    for (int channel = 0; channel < 2; ++channel)
        for (int sample = 0; sample < size; ++sample)
            audio.setSample(channel, sample, 0.75f);
    processor.processBlock(audio, midi);
    require(midi.isEmpty(), "Instrument emitted MIDI.");
    Samples result(static_cast<std::size_t>(size));
    for (int sample = 0; sample < size; ++sample)
    {
        const float value = audio.getSample(0, sample);
        require(std::isfinite(value) && std::abs(value) <= 0.80001f, "Invalid or unbounded output.");
        require(value == audio.getSample(1, sample), "Stereo channels differed.");
        result[static_cast<std::size_t>(sample)] = value;
    }
    return result;
}

bool silent(const Samples& samples)
{
    return std::all_of(samples.begin(), samples.end(), [](float value) { return value == 0.0f; });
}

double rms(const Samples& samples)
{
    double power = 0.0;
    for (const auto value : samples)
        power += static_cast<double>(value) * value;
    return std::sqrt(power / static_cast<double>(samples.size()));
}

double magnitude(const Samples& samples, double frequency, double rate)
{
    double real = 0.0;
    double imaginary = 0.0;
    for (std::size_t i = 0; i < samples.size(); ++i)
    {
        const auto angle = 2.0 * juce::MathConstants<double>::pi * frequency * static_cast<double>(i) / rate;
        real += samples[i] * std::cos(angle);
        imaginary += samples[i] * std::sin(angle);
    }
    return 2.0 * std::hypot(real, imaginary) / static_cast<double>(samples.size());
}

juce::MidiBuffer noteOn(int note, int channel = 1, float velocity = 1.0f, int position = 0)
{
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(channel, note, velocity), position);
    return midi;
}

juce::MidiBuffer noteOff(int note, int channel = 1, int position = 0)
{
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOff(channel, note), position);
    return midi;
}

void checkPitchAndFades(SynthesizerAudioProcessor& processor, double rate)
{
    const int attack = static_cast<int>(std::ceil(rate * 0.003));
    const int release = static_cast<int>(std::ceil(rate * 0.005));
    processor.reset();
    const auto onset = render(processor, 1024, noteOn(69, 1, 1.0f, 47));
    require(std::all_of(onset.begin(), onset.begin() + 47, [](float value) { return value == 0.0f; }),
            "Note started before its MIDI sample position.");
    require(!silent(Samples(onset.begin() + 47, onset.end())), "Held note was silent.");

    // Compare the actual onset to an identical settled oscillator at the same phase.
    SynthesizerAudioProcessor reference;
    reference.prepareToPlay(rate, 1024);
    // One second is exactly 440 periods at every supported integer sample rate.
    render(reference, static_cast<int>(rate), noteOn(69));
    const auto settled = render(reference, attack + 8);
    processor.reset();
    const auto fade = render(processor, attack + 8, noteOn(69));
    for (int i = 1; i < attack + 8; ++i)
    {
        const double level = std::min(1.0, (i + 1.0) / attack);
        require(std::abs(fade[static_cast<std::size_t>(i)] - settled[static_cast<std::size_t>(i)] * level) < 0.000002,
                "Fixed attack duration or shape differed.");
    }

    render(processor, static_cast<int>(rate));
    const auto held = render(processor, static_cast<int>(rate));
    require(magnitude(held, 440.0, rate) > 0.05, "A4 fundamental is missing.");
    require(magnitude(held, 430.0, rate) < 0.001 && magnitude(held, 450.0, rate) < 0.001,
            "A4 tuning is incorrect.");
    const double fundamental = magnitude(held, 440.0, rate);
    require(magnitude(held, 880.0, rate) / fundamental > 0.45
                && magnitude(held, 880.0, rate) / fundamental < 0.55
                && magnitude(held, 1320.0, rate) / fundamental > 0.28
                && magnitude(held, 1320.0, rate) / fundamental < 0.38,
            "Expected saw harmonic content.");

    // These processors now have matching phase and full level.
    processor.reset();
    reference.reset();
    render(processor, 1024, noteOn(69));
    render(reference, 1024, noteOn(69));
    const auto unfaded = render(reference, release + 32);
    const auto ending = render(processor, release + 32, noteOff(69, 1, 17));
    for (int i = 0; i < release + 32; ++i)
    {
        const double level = i < 17 ? 1.0 : std::max(0.0, 1.0 - (i - 16.0) / release);
        require(std::abs(ending[static_cast<std::size_t>(i)] - unfaded[static_cast<std::size_t>(i)] * level) < 0.000002,
                "Note-off timing or fixed release duration differed.");
    }
    require(silent(render(processor, 1024)), "Note remained active after release.");

    processor.reset();
    render(processor, 1024, noteOn(69, 1, 0.5f));
    const auto quiet = render(processor, static_cast<int>(rate));
    require(rms(quiet) / rms(held) > 0.49 && rms(quiet) / rms(held) < 0.51, "Velocity response differed.");
    juce::MidiBuffer zeroVelocity;
    zeroVelocity.addEvent(juce::MidiMessage::noteOn(1, 69, static_cast<juce::uint8>(0)), 0);
    require(silent(render(processor, release + 32, zeroVelocity)) == false, "Release tail disappeared.");
    require(silent(render(processor, 64)), "Zero-velocity note-on did not release the note.");

    // The entire MIDI pitch range must remain valid, including both endpoints.
    for (int note = 0; note < 128; ++note)
    {
        processor.reset();
        render(processor, 1024, noteOn(note));
        require(!silent(render(processor, 1024)), "A supported note was silent.");
        render(processor, release + 8, noteOff(note));
        require(silent(render(processor, 64)), "A supported note failed to release.");
    }
}

void checkVoices(SynthesizerAudioProcessor& processor)
{
    // Each note-off releases the oldest still-held instance of the same pitch.
    processor.reset();
    auto repeated = noteOn(69);
    repeated.addEvent(juce::MidiMessage::noteOn(1, 69, 0.25f), 0);
    render(processor, 1024, repeated);
    render(processor, 1024, noteOff(69));
    const auto remaining = render(processor, 48000);
    SynthesizerAudioProcessor quietReference;
    quietReference.prepareToPlay(48000.0, 1024);
    render(quietReference, 2048, noteOn(69, 1, 0.25f));
    require(remaining == render(quietReference, 48000), "Repeated pitches did not release in FIFO order.");

    processor.reset();
    juce::MidiBuffer midi;
    midi.addEvent(juce::MidiMessage::noteOn(1, 60, 1.0f), 0);
    midi.addEvent(juce::MidiMessage::noteOn(1, 60, 1.0f), 33);
    midi.addEvent(juce::MidiMessage::noteOn(2, 60, 1.0f), 66);
    render(processor, 1024, midi);
    render(processor, 1024, noteOff(60));
    require(!silent(render(processor, 1024)), "One note-off stopped overlapping notes.");
    render(processor, 1024, noteOff(60));
    require(!silent(render(processor, 1024)), "Note-off affected another channel.");
    render(processor, 1024, noteOff(60, 2));
    require(silent(render(processor, 64)), "Overlapping notes remained stuck.");

    // Full-velocity eight-note unison reaches the worst-case coherent sum.
    processor.reset();
    midi.clear();
    for (int i = 0; i < 8; ++i)
        midi.addEvent(juce::MidiMessage::noteOn(1, 69, 1.0f), 0);
    render(processor, 1024, midi);
    const auto unison = render(processor, 1024);
    require(rms(unison) > 0.4, "Expected eight simultaneous full-level voices.");
    midi.clear();
    midi.addEvent(juce::MidiMessage::allNotesOff(1), 37);
    const auto allOff = render(processor, 1024, midi);
    require(!silent(Samples(allOff.begin(), allOff.begin() + 37)), "All-notes-off arrived early.");
    require(silent(render(processor, 64)), "All-notes-off left held voices.");

    // Ninth note steals the oldest held pitch; its replacement arrives after a
    // bounded 1 ms fade. Removing notes 61..67 must leave note 72 alone.
    processor.reset();
    midi.clear();
    for (int note = 60; note < 68; ++note)
        midi.addEvent(juce::MidiMessage::noteOn(1, note, 1.0f), 0);
    render(processor, 1024, midi);
    render(processor, 1024, noteOn(72));
    midi.clear();
    for (int note = 61; note < 68; ++note)
        midi.addEvent(juce::MidiMessage::noteOff(1, note), 0);
    render(processor, 1024, midi);
    const auto survivor = render(processor, 48000);
    require(magnitude(survivor, juce::MidiMessage::getMidiNoteInHertz(60), 48000.0) < 0.001,
            "Oldest held voice was not stolen.");
    require(magnitude(survivor, juce::MidiMessage::getMidiNoteInHertz(72), 48000.0) > 0.05,
            "Replacement note did not survive voice stealing.");
    render(processor, 1024, noteOff(72));
    require(silent(render(processor, 64)), "Stolen/replacement voices remained stuck.");

    // A note released before its stolen slot is ready must never start later.
    processor.reset();
    midi.clear();
    for (int note = 60; note < 68; ++note)
        midi.addEvent(juce::MidiMessage::noteOn(1, note, 1.0f), 0);
    render(processor, 1024, midi);
    midi = noteOn(72);
    midi.addEvent(juce::MidiMessage::noteOff(1, 72), 1);
    for (int note = 60; note < 68; ++note)
        midi.addEvent(juce::MidiMessage::noteOff(1, note), 2);
    render(processor, 1024, midi);
    require(silent(render(processor, 1024)), "Cancelled queued note started later.");

    // Prefer an already-releasing slot over stealing a held note.
    processor.reset();
    midi.clear();
    for (int note = 60; note < 68; ++note)
        midi.addEvent(juce::MidiMessage::noteOn(1, note, 1.0f), 0);
    render(processor, 1024, midi);
    midi = noteOff(67);
    midi.addEvent(juce::MidiMessage::noteOn(1, 72, 1.0f), 1);
    render(processor, 1024, midi);
    midi.clear();
    for (int note = 61; note < 67; ++note)
        midi.addEvent(juce::MidiMessage::noteOff(1, note), 0);
    midi.addEvent(juce::MidiMessage::noteOff(1, 72), 0);
    render(processor, 1024, midi);
    require(!silent(render(processor, 1024)), "Releasing-voice preference stole the oldest held note.");
    render(processor, 1024, noteOff(60));
    require(silent(render(processor, 64)), "Reused released slot remained stuck.");

    // Dense bursts exhaust both sounding and queued slots and finish in silence.
    for (int iteration = 0; iteration < 20; ++iteration)
    {
        midi.clear();
        for (int note = 0; note < 128; ++note)
            midi.addEvent(juce::MidiMessage::noteOn(1 + note % 2, note, 1.0f), note % 16);
        render(processor, 64, midi);
        midi.clear();
        for (int note = 0; note < 128; ++note)
            midi.addEvent(juce::MidiMessage::noteOff(1 + note % 2, note), note % 16);
        render(processor, 1024, midi);
        require(silent(render(processor, 64)), "Dense MIDI burst left stuck notes.");
    }

    processor.reset();
    render(processor, 1024, noteOn(60));
    render(processor, 1024, noteOn(60, 2));
    midi.clear();
    midi.addEvent(juce::MidiMessage::allSoundOff(1), 29);
    render(processor, 1024, midi);
    require(!silent(render(processor, 64)), "All-sound-off affected another channel.");
    midi.clear();
    midi.addEvent(juce::MidiMessage::allSoundOff(2), 29);
    const auto panic = render(processor, 64, midi);
    require(silent(Samples(panic.begin() + 29, panic.end())), "All-sound-off was not immediate.");
}

Samples renderSequence(int blockSize)
{
    SynthesizerAudioProcessor processor;
    processor.prepareToPlay(48000.0, 1024);
    struct Event { int position; juce::MidiMessage message; };
    const std::array<Event, 6> events {{
        { 0, juce::MidiMessage::noteOn(1, 69, 0.8f) },
        { 64, juce::MidiMessage::noteOn(1, 72, 1.0f) },
        { 255, juce::MidiMessage::noteOff(1, 69) },
        { 512, juce::MidiMessage::noteOn(2, 60, 0.5f) },
        { 1024, juce::MidiMessage::allNotesOff(1) },
        { 1400, juce::MidiMessage::allNotesOff(2) }
    }};
    Samples result;
    for (int start = 0; start < 2048; start += blockSize)
    {
        const int size = std::min(blockSize, 2048 - start);
        juce::MidiBuffer midi;
        for (const auto& event : events)
            if (event.position >= start && event.position < start + size)
                midi.addEvent(event.message, event.position - start);
        const auto block = render(processor, size, midi);
        result.insert(result.end(), block.begin(), block.end());
    }
    require(silent(Samples(result.end() - 64, result.end())), "Sequence did not end in silence.");
    return result;
}

void checkAliasReduction()
{
    constexpr int count = 4096;
    constexpr double increment = 9000.0 / 48000.0;
    laz::dsp::SawOscillator oscillator;
    oscillator.start(increment);
    Samples corrected(count), naive(count);
    double phase = 0.0;
    for (int i = 0; i < count; ++i)
    {
        corrected[static_cast<std::size_t>(i)] = oscillator.nextSample();
        naive[static_cast<std::size_t>(i)] = static_cast<float>(2.0 * phase - 1.0);
        phase += increment;
        phase -= std::floor(phase);
    }
    double correctedAlias = 0.0, naiveAlias = 0.0;
    for (const double frequency : { 3000.0, 12000.0, 21000.0 })
    {
        correctedAlias += std::pow(magnitude(corrected, frequency, 48000.0), 2.0);
        naiveAlias += std::pow(magnitude(naive, frequency, 48000.0), 2.0);
    }
    require(correctedAlias < naiveAlias * 0.3, "Saw discontinuity correction did not reduce folded harmonics.");
    std::cout << "Alias check at 9 kHz: corrected/naive folded-bin power = " << correctedAlias / naiveAlias << '\n';
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

        // No audio device or visible window is required for these product checks.
        for (const auto rate : std::array<double, 3> { 44100.0, 48000.0, 96000.0 })
        {
            processor.setRateAndBufferSizeDetails(rate, 1024);
            processor.prepareToPlay(rate, 1024);
            for (const auto size : std::array<int, 5> { 0, 1, 64, 256, 1024 })
            {
                require(silent(render(processor, size)), "Idle dirty buffer did not become silent.");
                render(processor, size, noteOn(69));
                render(processor, size, noteOff(69));
                render(processor, 1024);
                require(silent(render(processor, size)), "Voice remained active after release.");
            }
            checkPitchAndFades(processor, rate);
            render(processor, 1024, noteOn(69));
            processor.releaseResources();
            require(silent(render(processor, 64)), "releaseResources left active notes.");
            processor.prepareToPlay(rate, 1024);
            render(processor, 1024, noteOn(69));
            processor.prepareToPlay(rate, 1024);
            require(silent(render(processor, 64)), "prepareToPlay did not reset voices.");
        }

        processor.prepareToPlay(48000.0, 1024);
        checkVoices(processor);
        const auto sequence = renderSequence(1024);
        for (const auto size : { 1, 64, 256 })
            require(sequence == renderSequence(size), "MIDI/audio depended on block size.");
        checkAliasReduction();

        // Empty blocks and boundary events update MIDI state without inventing
        // audio samples. A note scheduled at the block end starts in the next block.
        processor.reset();
        render(processor, 0, noteOn(69));
        require(!silent(render(processor, 64)), "Empty-block MIDI was discarded.");
        processor.reset();
        require(silent(render(processor, 64, noteOn(69, 1, 1.0f, 64))), "Boundary note started early.");
        require(!silent(render(processor, 64)), "Boundary note failed to start.");

        // A long held note remains bounded with changing block sizes.
        processor.reset();
        render(processor, 1024, noteOn(127));
        for (int block = 0; block < 3000; ++block)
            render(processor, block % 2 == 0 ? 1024 : 1);
        render(processor, 1024, noteOff(127));
        require(silent(render(processor, 64)), "Long held note failed to release.");

        // Unsupported expression and a long SysEx message must leave pitch and
        // voice state alone; processing never constructs owning MIDI messages.
        processor.reset();
        juce::MidiBuffer ignored;
        std::array<juce::uint8, 512> sysex {};
        ignored.addEvent(juce::MidiMessage::createSysExMessage(sysex.data(), static_cast<int>(sysex.size())), 0);
        ignored.addEvent(juce::MidiMessage::pitchWheel(1, 16383), 0);
        ignored.addEvent(juce::MidiMessage::controllerEvent(1, 64, 127), 0);
        require(silent(render(processor, 64, ignored)), "Unsupported MIDI created a note.");
        render(processor, 1024, noteOn(69));
        processor.reset();
        require(silent(render(processor, 64)), "reset left held notes.");
        render(processor, 1024, noteOn(69));
        juce::MemoryBlock state;
        processor.getStateInformation(state);
        SynthesizerAudioProcessor restored;
        restored.prepareToPlay(48000.0, 1024);
        restored.setStateInformation(state.getData(), static_cast<int>(state.getSize()));
        require(silent(render(restored, 64)), "State restored live notes.");

        for (int iteration = 0; iteration < 3; ++iteration)
        {
            std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
            require(editor != nullptr && editor->getWidth() == 560 && editor->getHeight() == 320,
                    "Editor could not be recreated at its expected size.");
            juce::Image preview(juce::Image::ARGB, editor->getWidth(), editor->getHeight(), true);
            juce::Graphics graphics(preview);
            editor->paint(graphics);
        }

        std::cout << "PASS: saw pitch/harmonics/alias reduction, fades/velocity, sample-position MIDI,\n"
                     "eight voices/stealing/channels/panic, three rates/block sizes, lifecycle/state/editor.\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "FAIL: " << error.what() << '\n';
        return 1;
    }
}
