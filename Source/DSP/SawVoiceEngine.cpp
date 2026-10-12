#include "SawVoiceEngine.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace laz::dsp
{
void SawVoiceEngine::prepare(double sampleRate) noexcept
{
    if (!std::isfinite(sampleRate) || sampleRate <= 0.0)
        sampleRate = 48000.0;

    attackSamples = std::max(1, static_cast<int>(std::ceil(sampleRate * attackSeconds)));
    releaseSamples = std::max(1, static_cast<int>(std::ceil(sampleRate * releaseSeconds)));
    stealSamples = std::max(1, static_cast<int>(std::ceil(sampleRate * stealSeconds)));
    for (int note = 0; note < 128; ++note)
        increments[static_cast<std::size_t>(note)] = 440.0 * std::exp2((note - 69) / 12.0) / sampleRate;
    reset();
}

void SawVoiceEngine::reset() noexcept
{
    voices = {};
    nextOrder = 0;
}

void SawVoiceEngine::startVoice(Voice& voice, Note note) noexcept
{
    voice.current = note;
    voice.pending = {};
    voice.oscillator.start(increments[static_cast<std::size_t>(note.number)]);
    voice.level = 0.0;
    voice.step = 1.0 / attackSamples;
    voice.remaining = attackSamples;
    voice.stage = Stage::attack;
}

void SawVoiceEngine::fadeVoice(Voice& voice, int samples) noexcept
{
    if (voice.stage == Stage::idle)
        return;
    // Further steals shorten a fade if needed, but never restart or extend it.
    if (voice.stage == Stage::release)
        samples = std::min(samples, voice.remaining);
    voice.remaining = samples;
    voice.step = voice.level / samples;
    voice.stage = Stage::release;
}

void SawVoiceEngine::noteOn(int channel, int note, int velocity) noexcept
{
    const Note incoming { note, channel, velocity / 127.0f, ++nextOrder };
    for (auto& voice : voices)
    {
        if (voice.stage == Stage::idle)
        {
            startVoice(voice, incoming);
            return;
        }
    }

    // Prefer a voice already releasing (without a queued replacement), then the
    // oldest held note, then the oldest queued note if every voice is stealing.
    Voice* selected = nullptr;
    int bestPriority = 3;
    auto oldest = std::numeric_limits<std::uint64_t>::max();
    for (auto& voice : voices)
    {
        const int priority = voice.pending.number >= 0 ? 2 : voice.stage == Stage::release ? 0 : 1;
        const auto order = voice.pending.number >= 0 ? voice.pending.order : voice.current.order;
        if (priority < bestPriority || (priority == bestPriority && order < oldest))
        {
            selected = &voice;
            bestPriority = priority;
            oldest = order;
        }
    }
    selected->pending = incoming;
    fadeVoice(*selected, stealSamples);
}

void SawVoiceEngine::noteOff(int channel, int note) noexcept
{
    Voice* selected = nullptr;
    bool pending = false;
    auto oldest = std::numeric_limits<std::uint64_t>::max();
    for (auto& voice : voices)
    {
        const bool held = voice.stage == Stage::attack || voice.stage == Stage::held;
        if (held && voice.current.channel == channel && voice.current.number == note
            && voice.current.order < oldest)
        {
            selected = &voice;
            pending = false;
            oldest = voice.current.order;
        }
        if (voice.pending.channel == channel && voice.pending.number == note && voice.pending.order < oldest)
        {
            selected = &voice;
            pending = true;
            oldest = voice.pending.order;
        }
    }
    if (selected != nullptr)
    {
        if (pending)
            selected->pending = {};
        else
            fadeVoice(*selected, releaseSamples);
    }
}

void SawVoiceEngine::stopChannel(int channel, bool immediately) noexcept
{
    for (auto& voice : voices)
    {
        if (voice.pending.channel == channel)
            voice.pending = {};
        if (voice.current.channel != channel)
            continue;
        if (immediately)
        {
            const auto pending = voice.pending;
            voice = {};
            if (pending.number >= 0)
                startVoice(voice, pending);
        }
        else if (voice.stage == Stage::attack || voice.stage == Stage::held)
        {
            fadeVoice(voice, releaseSamples);
        }
    }
}

void SawVoiceEngine::handleMidi(const std::uint8_t* data, int size) noexcept
{
    // Use JUCE's non-owning message view; copying long SysEx messages may allocate.
    if (data == nullptr || size != 3 || data[1] > 127 || data[2] > 127)
        return;
    const int channel = (data[0] & 0x0f) + 1;
    switch (data[0] & 0xf0)
    {
        case 0x90:
            if (data[2] != 0)
                noteOn(channel, data[1], data[2]);
            else
                noteOff(channel, data[1]);
            break;
        case 0x80: noteOff(channel, data[1]); break;
        case 0xb0:
            if (data[1] == 120 || data[1] == 123)
                stopChannel(channel, data[1] == 120);
            break;
        default: break;
    }
}

float SawVoiceEngine::nextSample(Voice& voice) noexcept
{
    if (voice.stage == Stage::idle)
        return 0.0f;
    if (voice.stage == Stage::attack)
    {
        voice.level += voice.step;
        if (--voice.remaining == 0)
        {
            voice.level = 1.0;
            voice.stage = Stage::held;
        }
    }
    else if (voice.stage == Stage::release)
    {
        voice.level = std::max(0.0, voice.level - voice.step);
        if (--voice.remaining == 0)
            voice.level = 0.0;
    }

    const float output = voice.oscillator.nextSample()
        * static_cast<float>(voice.level) * voice.current.velocity * voiceGain;
    if (voice.stage == Stage::release && voice.remaining == 0)
    {
        const auto pending = voice.pending;
        voice = {};
        if (pending.number >= 0)
            startVoice(voice, pending);
    }
    return output;
}

void SawVoiceEngine::render(float* left, float* right, int sampleCount) noexcept
{
    for (int sample = 0; sample < sampleCount; ++sample)
    {
        float output = 0.0f;
        for (auto& voice : voices)
            output += nextSample(voice);
        left[sample] = output;
        right[sample] = output;
    }
}
}
