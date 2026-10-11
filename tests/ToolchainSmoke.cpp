#include <windows.h>

#include <array>
#include <atomic>
#include <cmath>
#include <iostream>

#if !defined(_WIN64) || !defined(_M_X64)
#error This check must be compiled for Windows x64.
#endif

int main()
{
    static_assert(sizeof(void*) == 8, "A 64-bit executable is required.");
    LARGE_INTEGER frequency {};
    if (QueryPerformanceFrequency(&frequency) == 0 || frequency.QuadPart <= 0)
    {
        std::cerr << "Windows SDK/runtime check failed.\n";
        return 1;
    }

    std::atomic<float> parameter { 0.25f };
    if (!parameter.is_lock_free())
    {
        std::cerr << "Float atomics require locks on this target.\n";
        return 1;
    }

    std::array<float, 480> samples {};
    double energy = 0.0;
    constexpr auto twoPi = 6.28318530717958647692;
    for (std::size_t i = 0; i < samples.size(); ++i)
    {
        samples[i] = parameter.load(std::memory_order_relaxed)
            * static_cast<float>(std::sin(twoPi * 440.0 * static_cast<double>(i) / 48000.0));
        if (!std::isfinite(samples[i]))
            return 1;
        energy += static_cast<double>(samples[i]) * samples[i];
    }
    if (energy <= 0.0)
        return 1;

    std::cout << "PASS: Windows x64, MSVC " << _MSC_FULL_VER
              << ", C++17, Windows SDK/runtime, lock-free float atomics, finite offline samples.\n";
    return 0;
}
