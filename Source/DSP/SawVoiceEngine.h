#pragma once

#include "SawOscillator.h"

#include <array>
#include <cstdint>

namespace laz::dsp
{
class SawVoiceEngine
{
public:
    static constexpr int voiceCount = 8;
    static constexpr float voiceGain = 0.1f; // Eight full-velocity voices stay below 0.8.
    static constexpr double attackSeconds = 0.003;
    static constexpr double releaseSeconds = 0.005;
    static constexpr double stealSeconds = 0.001;

    void prepare(double sampleRate) noexcept;
    void reset() noexcept;
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

    enum class Stage { idle, attack, held, release };

    struct Voice
    {
        SawOscillator oscillator;
        Note current;
        Note pending;
        Stage stage = Stage::idle;
        double level = 0.0;
        double step = 0.0;
        int remaining = 0;
    };

    void startVoice(Voice& voice, Note note) noexcept;
    void fadeVoice(Voice& voice, int samples) noexcept;
    void noteOn(int channel, int note, int velocity) noexcept;
    void noteOff(int channel, int note) noexcept;
    void stopChannel(int channel, bool immediately) noexcept;
    float nextSample(Voice& voice) noexcept;

    std::array<Voice, voiceCount> voices {};
    std::array<double, 128> increments {};
    std::uint64_t nextOrder = 0;
    int attackSamples = 144;
    int releaseSamples = 240;
    int stealSamples = 48;
};
}
