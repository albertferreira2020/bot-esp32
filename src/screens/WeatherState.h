#pragma once
#include "core/IAppState.h"
#include "core/Scheduler.h"

// Temperatura, condição, umidade e vento (Open-Meteo via Scheduler). Nuvens
// animadas com ícones proceduralmente desenhados (sem asset).
class WeatherState : public IAppState {
   public:
    explicit WeatherState(const Scheduler& scheduler) : scheduler_(scheduler) {}

    AppStateId id() const override { return AppStateId::Weather; }

    void onEnter() override { elapsedMs_ = 0; }
    void update(uint32_t dtMs, uint32_t nowMs) override { elapsedMs_ += dtMs; (void)nowMs; }
    void render(DisplayManager& display) override;
    bool finished() const override;

   private:
    const Scheduler& scheduler_;
    uint32_t elapsedMs_ = 0;
};
