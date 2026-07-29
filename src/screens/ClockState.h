#pragma once
#include "core/IAppState.h"

// "16:45 / terca-feira / 29 julho", com uma pequena animação "digital"
// (piscar dos dois-pontos + leve glitch de dígitos).
class ClockState : public IAppState {
   public:
    AppStateId id() const override { return AppStateId::Clock; }

    void onEnter() override { elapsedMs_ = 0; }
    void update(uint32_t dtMs, uint32_t nowMs) override { elapsedMs_ += dtMs; (void)nowMs; }
    void render(DisplayManager& display) override;
    bool finished() const override;

   private:
    uint32_t elapsedMs_ = 0;
};
