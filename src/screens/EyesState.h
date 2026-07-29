#pragma once
#include "core/IAppState.h"
#include "robot/RobotFace.h"

// Tela de idle: só os olhos, sempre vivos (respiração, piscar, olhar ao
// redor, partículas). RobotFace é compartilhado (dono em main.cpp) para
// manter continuidade de humor entre as aparições no ciclo.
class EyesState : public IAppState {
   public:
    explicit EyesState(RobotFace& face) : face_(face) {}

    AppStateId id() const override { return AppStateId::Eyes; }

    void onEnter() override { elapsedMs_ = 0; }

    void update(uint32_t dtMs, uint32_t nowMs) override {
        elapsedMs_ += dtMs;
        face_.update(dtMs, nowMs);
    }

    void render(DisplayManager& display) override {
        face_.render(display);
        Rect r = face_.dirtyRect();
        display.pushRegion(r.x, r.y, r.w, r.h);
    }

    bool finished() const override;

   private:
    RobotFace& face_;
    uint32_t elapsedMs_ = 0;
};
