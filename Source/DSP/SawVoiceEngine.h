#pragma once

#include "SawOscillator.h"
#include "AmpEnvelope.h"

#include <array>
#include <cstdint>

namespace laz::dsp
{
class SawVoiceEngine
{
public:
    static constexpr int voiceCount = 8;
    static constexpr float voiceGain = 0.1f; // Eight full-velocity voices stay below 0.8.
    static constexpr double stealSeconds = 0.001;

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
    void setEnvelopeParameters(AmpEnvelope::Parameters parameters) noexcept;
    void handleMidi(const std::uint8_t* data, int size) noexcept;
    void render(float* left, float* right, int sampleCount) noexcept;

private:
    struct Note
    {
        int number = -1;
        int channel = 0;
        float velocity = 0.0f;
        std::uint64_t order = 0;
    };

    struct Voice
    {
        SawOscillator oscillator;
        AmpEnvelope envelope;
        Note current;
        Note pending;
        bool stealing = false;
    };

    void startVoice(Voice& voice, Note note) noexcept;
    void noteOn(int channel, int note, int velocity) noexcept;
    void noteOff(int channel, int note) noexcept;
    void stopChannel(int channel, bool immediately) noexcept;
    float nextSample(Voice& voice) noexcept;

    std::array<Voice, voiceCount> voices {};
    std::array<double, 128> increments {};
    std::uint64_t nextOrder = 0;
    AmpEnvelope::Parameters envelopeParameters;
    double rate = 48000.0;
    int stealSamples = 48;
};
}
