#include "animation/Easing.h"
#include <cmath>

namespace {
constexpr float kPi = 3.14159265358979323846f;

float clamp01(float t) { return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t); }

float easeInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

float easeOutElastic(float t) {
    constexpr float c4 = (2.0f * kPi) / 3.0f;
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
}

float easeOutBounce(float t) {
    constexpr float n1 = 7.5625f;
    constexpr float d1 = 2.75f;
    if (t < 1.0f / d1) {
        return n1 * t * t;
    } else if (t < 2.0f / d1) {
        t -= 1.5f / d1;
        return n1 * t * t + 0.75f;
    } else if (t < 2.5f / d1) {
        t -= 2.25f / d1;
        return n1 * t * t + 0.9375f;
    } else {
        t -= 2.625f / d1;
        return n1 * t * t + 0.984375f;
    }
}

// Aproximação de mola: oscilação amortecida que assenta em 1.0.
float springEase(float t) {
    if (t >= 1.0f) return 1.0f;
    float decay = std::exp(-6.0f * t);
    return 1.0f - decay * std::cos(t * kPi * 4.5f);
}
}  // namespace

float ease(EasingType type, float t) {
    switch (type) {
        case EasingType::Linear: return clamp01(t);
        case EasingType::EaseInOut: return easeInOutCubic(clamp01(t));
        case EasingType::Elastic: return easeOutElastic(t);
        case EasingType::Bounce: return easeOutBounce(clamp01(t));
        case EasingType::Spring: return springEase(t);
    }
    return clamp01(t);
}
