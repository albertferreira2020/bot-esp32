#pragma once
#include <cstdint>
#include "animation/Easing.h"

// Valor float animável com easing + duração. Base de toda animação suave do
// projeto (posição de pupila, escala do peito ao respirar, fade de tela,
// rotação do logo do Bitcoin, etc).
class Tween {
   public:
    void start(float from, float to, uint32_t durationMs, EasingType easing = EasingType::EaseInOut) {
        from_ = from;
        to_ = to;
        durationMs_ = durationMs == 0 ? 1 : durationMs;
        easing_ = easing;
        elapsedMs_ = 0;
        running_ = true;
    }

    void update(uint32_t dtMs) {
        if (!running_) return;
        elapsedMs_ += dtMs;
        if (elapsedMs_ >= durationMs_) {
            elapsedMs_ = durationMs_;
            running_ = false;
        }
    }

    float value() const {
        float t = (float)elapsedMs_ / (float)durationMs_;
        return from_ + (to_ - from_) * ease(easing_, t);
    }

    bool finished() const { return !running_; }

    void snapTo(float v) {
        from_ = to_ = v;
        elapsedMs_ = durationMs_;
        running_ = false;
    }

   private:
    float from_ = 0.0f, to_ = 0.0f;
    uint32_t durationMs_ = 1, elapsedMs_ = 0;
    EasingType easing_ = EasingType::EaseInOut;
    bool running_ = false;
};
