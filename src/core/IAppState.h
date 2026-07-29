#pragma once
#include <cstdint>

class DisplayManager;

enum class AppStateId {
    Boot,
    Eyes,
    Transition,
    Bitcoin,
    Weather,
    Rain,
    Clock,
    WifiSearch,
};

// Contrato comum de qualquer tela/fluxo. Cada novo fluxo do robô é só mais
// uma implementação disso — a StateMachine não precisa saber de nada além
// desta interface.
class IAppState {
   public:
    virtual ~IAppState() = default;

    virtual AppStateId id() const = 0;

    virtual void onEnter() {}
    virtual void onExit() {}

    virtual void update(uint32_t dtMs, uint32_t nowMs) = 0;
    virtual void render(DisplayManager& display) = 0;

    // Quando true, a StateMachine considera a tela concluída e decide a
    // próxima (via transição, salvo estados urgentes como WifiSearch).
    virtual bool finished() const { return false; }
};
