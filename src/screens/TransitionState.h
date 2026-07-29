#pragma once
#include "core/IAppState.h"
#include "media/MjpegPlayer.h"

// Vídeo holográfico entre duas telas (5s). Toca data/videos/transition/ se
// existir; senão, glitch + scanlines + halo pulsante proceduralmente. Não
// sabe (nem precisa saber) qual é o próximo estado — a StateMachine decide.
class TransitionState : public IAppState {
   public:
    explicit TransitionState(DisplayManager& display) : display_(display) {}

    AppStateId id() const override { return AppStateId::Transition; }

    void onEnter() override;
    void update(uint32_t dtMs, uint32_t nowMs) override;
    void render(DisplayManager& display) override;
    bool finished() const override;

   private:
    DisplayManager& display_;
    MjpegPlayer mjpeg_;
    bool usingVideo_ = false;
    uint32_t elapsedMs_ = 0;
    uint32_t nextGlitchAt_ = 0;
};
