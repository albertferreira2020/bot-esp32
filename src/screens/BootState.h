#pragma once
#include "core/IAppState.h"
#include "media/MjpegPlayer.h"

// "Vídeo de abertura". Toca data/videos/boot/ se existir; senão, um efeito
// holográfico procedural (halo crescendo + scanlines + glitch) na mesma
// duração. Ao terminar, a StateMachine vai direto para Eyes (sem transição
// extra — este já É o vídeo de abertura).
class BootState : public IAppState {
   public:
    explicit BootState(DisplayManager& display) : display_(display) {}

    AppStateId id() const override { return AppStateId::Boot; }

    void onEnter() override;
    void update(uint32_t dtMs, uint32_t nowMs) override;
    void render(DisplayManager& display) override;
    bool finished() const override;

   private:
    DisplayManager& display_;
    MjpegPlayer mjpeg_;
    bool usingVideo_ = false;
    uint32_t elapsedMs_ = 0;
};
