#pragma once
#include "core/IAppState.h"
#include "core/Scheduler.h"
#include "media/MjpegPlayer.h"

// Só entra no ciclo quando a previsão indica chuva (ver StateMachine). Toca
// data/videos/rain/ se existir; senão, chuva procedural (linhas caindo).
// Depois de alguns segundos, sobrepõe o card "Vai chover".
class RainState : public IAppState {
   public:
    explicit RainState(DisplayManager& display, const Scheduler& scheduler)
        : display_(display), scheduler_(scheduler) {}

    AppStateId id() const override { return AppStateId::Rain; }

    void onEnter() override;
    void update(uint32_t dtMs, uint32_t nowMs) override;
    void render(DisplayManager& display) override;
    bool finished() const override;

   private:
    struct Drop {
        float x, y, speed, length;
    };
    static constexpr int kDropCount = 40;

    DisplayManager& display_;
    const Scheduler& scheduler_;
    MjpegPlayer mjpeg_;
    bool usingVideo_ = false;
    uint32_t elapsedMs_ = 0;
    Drop drops_[kDropCount];
};
