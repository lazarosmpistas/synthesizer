#pragma once

#include <algorithm>
#include <cmath>

namespace laz::dsp
{
// Linear ADSR with continuous ramps when a host changes a live parameter.
class AmpEnvelope
{
public:
    struct Parameters
    {
        double attack = 0.003;
        double decay = 0.1;
        double sustain = 1.0;
        double release = 0.005;
    };

    void prepare(double rate) noexcept
    {
        sampleRate = rate;
        stage = Stage::idle;
        level = 0.0;
        remaining = 0;
    }

    void setParameters(Parameters next) noexcept
    {
        if (stage == Stage::attack && next.attack != parameters.attack)
            ramp(Stage::attack, 1.0, samples(next.attack));
        else if (stage == Stage::decay
                 && (next.decay != parameters.decay || next.sustain != parameters.sustain))
            ramp(Stage::decay, next.sustain, samples(next.decay));
        else if (stage == Stage::sustain && next.sustain != parameters.sustain)
            ramp(Stage::sustain, next.sustain, samples(0.01));
        else if (stage == Stage::release && next.release != parameters.release)
            ramp(Stage::release, 0.0, samples(next.release));
        parameters = next;
    }

    void noteOn() noexcept
    {
        level = 0.0;
        ramp(Stage::attack, 1.0, samples(parameters.attack));
    }

    void noteOff() noexcept
    {
        if (isHeld())
            ramp(Stage::release, 0.0, samples(parameters.release));
    }

    void steal(int duration) noexcept
    {
        // A repeated steal can shorten a release, but must never prolong it.
        if (isReleasing())
            duration = std::min(duration, remaining);
        ramp(Stage::release, 0.0, duration);
    }

    bool isActive() const noexcept { return stage != Stage::idle; }
    bool isReleasing() const noexcept { return stage == Stage::release; }
    bool isHeld() const noexcept { return isActive() && !isReleasing(); }

    double nextSample() noexcept
    {
        if (remaining > 0)
        {
            level = std::clamp(level + step, 0.0, 1.0);
            if (--remaining == 0)
            {
                level = target;
                if (stage == Stage::attack)
                    ramp(Stage::decay, parameters.sustain, samples(parameters.decay));
                else if (stage == Stage::decay)
                    stage = Stage::sustain;
                else if (stage == Stage::release)
                    stage = Stage::idle;
            }
        }
        return level;
    }

private:
    enum class Stage { idle, attack, decay, sustain, release };

    int samples(double seconds) const noexcept
    {
        return std::max(1, static_cast<int>(std::ceil(sampleRate * seconds)));
    }

    void ramp(Stage next, double destination, int duration) noexcept
    {
        stage = next;
        target = destination;
        remaining = std::max(1, duration);
        step = (target - level) / remaining;
    }

    Parameters parameters;
    Stage stage = Stage::idle;
    double sampleRate = 48000.0;
    double level = 0.0;
    double target = 0.0;
    double step = 0.0;
    int remaining = 0;
};
}
