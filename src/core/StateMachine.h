#pragma once
#include <functional>
#include <vector>
#include "core/IAppState.h"

// Orquestra o ciclo contínuo do robô. Não conhece o conteúdo de cada tela —
// só a interface IAppState — então adicionar um novo fluxo é: implementar
// IAppState, registrar no ciclo (ou plugar como preempção) e pronto.
//
// Ciclo de conteúdo (cada item roa a duração da tela; TransitionState é
// sempre injetada entre dois itens de conteúdo):
//   Eyes -> Bitcoin -> Eyes -> Weather -> Eyes -> [Rain se previsão indicar]
//   -> Clock -> (volta pro início)
//
// WifiSearch tem prioridade: se o wifi cair, a StateMachine troca na hora
// (sem transição, é urgente) para o estado de "procurando sinal" e retoma o
// ciclo em Eyes assim que a conexão voltar.
class StateMachine {
   public:
    struct States {
        IAppState* boot;
        IAppState* eyes;
        IAppState* transition;
        IAppState* bitcoin;
        IAppState* weather;
        IAppState* rain;
        IAppState* clock;
        IAppState* wifiSearch;
    };

    // rainForecast: retorna true quando a previsão indica chuva (RainState
    // deve entrar no ciclo). Injetado para não acoplar a StateMachine à
    // WeatherClient/Scheduler diretamente.
    void begin(const States& states, std::function<bool()> rainForecast);

    void notifyWifiDown();
    void notifyWifiUp();

    void update(uint32_t dtMs, uint32_t nowMs);
    void render(DisplayManager& display);

    AppStateId currentId() const { return current_ ? current_->id() : AppStateId::Boot; }

   private:
    void enter(IAppState* state);
    void goToNextInCycle();
    IAppState* peekNextContentState();

    States states_{};
    std::function<bool()> rainForecast_;

    IAppState* current_ = nullptr;
    IAppState* pendingAfterTransition_ = nullptr;
    IAppState* resumeAfterWifi_ = nullptr;

    int cycleIndex_ = -1;  // índice do último estado de conteúdo mostrado
    bool wifiDown_ = false;
    bool bootDone_ = false;
};
