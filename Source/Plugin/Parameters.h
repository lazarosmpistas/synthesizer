#pragma once

#include <array>

namespace laz::parameters
{
struct Definition
{
    const char* id;
    const char* name;
    const char* unit;
    float minimum;
    float maximum;
    float defaultValue;
    float midpoint;
};

// These IDs and their meanings are part of the saved-project/automation contract.
inline constexpr std::array<Definition, 4> envelope {{
    { "ampAttackMs", "Attack", "ms", 1.0f, 5000.0f, 3.0f, 250.0f },
    { "ampDecayMs", "Decay", "ms", 1.0f, 5000.0f, 100.0f, 250.0f },
    { "ampSustain", "Sustain", "%", 0.0f, 1.0f, 1.0f, 0.5f },
    { "ampReleaseMs", "Release", "ms", 1.0f, 10000.0f, 5.0f, 500.0f }
}};
inline constexpr const char* stateType = "LAZ_SYNTH_STATE";
inline constexpr int stateVersion = 1;
inline constexpr double maximumTailSeconds = 10.0;
}
