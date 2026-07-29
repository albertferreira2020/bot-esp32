#pragma once
#include "animation/Tween.h"
#include "core/IAppState.h"
#include "core/Scheduler.h"

// Fluxo: logo sobe de baixo -> para no centro -> gira em 3D -> surge valor.
// Preço/variação vêm do Scheduler (polling assíncrono, nunca bloqueia aqui).
class BitcoinState : public IAppState {
   public:
    explicit BitcoinState(const Scheduler& scheduler) : scheduler_(scheduler) {}

    AppStateId id() const override { return AppStateId::Bitcoin; }

    void onEnter() override;
    void update(uint32_t dtMs, uint32_t nowMs) override;
    void render(DisplayManager& display) override;
    bool finished() const override;

   private:
    const Scheduler& scheduler_;
    uint32_t elapsedMs_ = 0;
    Tween riseY_;
    float spinAngle_ = 0.0f;
};
