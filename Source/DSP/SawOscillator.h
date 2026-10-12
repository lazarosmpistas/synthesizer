#pragma once

#include <algorithm>

namespace laz::dsp
{
// A rising saw with a two-sample polynomial correction around each phase wrap.
// PolyBLEP reduces the aliasing of the discontinuity; it is not an ideal bandlimit.
class SawOscillator
{
public:
    void start(double increment) noexcept
    {
        phase = 0.0;
        phaseIncrement = std::clamp(increment, 0.0, 0.49);
    }

    float nextSample() noexcept
    {
        double value = 2.0 * phase - 1.0;
        if (phaseIncrement > 0.0 && phase < phaseIncrement)
        {
            const auto t = phase / phaseIncrement;
            value -= 2.0 * t - t * t - 1.0;
        }
        else if (phaseIncrement > 0.0 && phase > 1.0 - phaseIncrement)
        {
            const auto t = (phase - 1.0) / phaseIncrement;
            value -= t * t + 2.0 * t + 1.0;
        }

        phase += phaseIncrement;
        if (phase >= 1.0)
            phase -= 1.0;
        return static_cast<float>(value);
    }

private:
    double phase = 0.0;
    double phaseIncrement = 0.0;
};
}
