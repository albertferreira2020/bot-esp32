#pragma once
#include "core/IAppState.h"
#include "robot/RobotFace.h"

// Preempção urgente quando o WiFi cai (ver StateMachine). Mostra o robô
// "procurando sinal": olhos olhando ao redor + um pequeno radar pulsante.
// finished() sempre false — quem tira este estado de cena é
// StateMachine::notifyWifiUp(), não a duração.
class WifiSearchState : public IAppState {
   public:
    explicit WifiSearchState(RobotFace& face) : face_(face) {}

    AppStateId id() const override { return AppStateId::WifiSearch; }

    void onEnter() override {}
    void update(uint32_t dtMs, uint32_t nowMs) override { face_.update(dtMs, nowMs); }
    void render(DisplayManager& display) override;
    bool finished() const override { return false; }

   private:
    RobotFace& face_;
};
