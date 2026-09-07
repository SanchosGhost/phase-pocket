#include <algorithm>
#include <cassert>
#include <cmath>
#include <complex>
#include <iostream>
#include <random>

static float gainFor(std::complex<float> bass, std::complex<float> kick, float tolerance, bool phaseAware)
{
    constexpr float eps = 1.0e-12f;
    const float pb = std::norm(bass), pk = std::norm(kick);
    const float x = phaseAware ? std::real(bass * std::conj(kick)) : 0.0f;
    const float target = std::max(std::abs(bass), std::abs(kick)) * tolerance;
    const float targetPower = target * target, predicted = pb + pk + 2.0f * x;
    if (predicted <= targetPower || pb <= eps) return 1.0f;
    const float d = std::max(0.0f, x * x + pb * (targetPower - pk));
    return std::clamp((-x + std::sqrt(d)) / (pb + eps), 0.0f, 1.0f);
}

int main()
{
    std::mt19937 rng(42); std::uniform_real_distribution<float> value(-2.0f, 2.0f);
    constexpr float tolerance = 1.122018454f;
    float worstError = 0.0f; int reduced = 0;
    for (int i = 0; i < 1000000; ++i)
    {
        std::complex<float> b(value(rng), value(rng)), k(value(rng), value(rng));
        const float g = gainFor(b, k, tolerance, true);
        const float target = std::max(std::abs(b), std::abs(k)) * tolerance;
        const float result = std::abs(k + g * b);
        worstError = std::max(worstError, result - target); if (g < 0.99999f) ++reduced;
        assert(g >= 0.0f && g <= 1.0f); assert(result <= target + 2.0e-5f);
    }
    std::cout << "PASS: 1,000,000 complex-bin cases\nReduced cases: " << reduced << "\nWorst target overshoot: " << worstError << "\n";
}
