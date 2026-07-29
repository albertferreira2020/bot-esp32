#include "core/StateMachine.h"

void StateMachine::begin(const States& states, std::function<bool()> rainForecast) {
    states_ = states;
    rainForecast_ = std::move(rainForecast);
    enter(states_.boot);
}

void StateMachine::notifyWifiDown() { wifiDown_ = true; }

void StateMachine::notifyWifiUp() {
    if (!wifiDown_) return;
    wifiDown_ = false;
    if (current_ == states_.wifiSearch) {
        IAppState* resume = resumeAfterWifi_ ? resumeAfterWifi_ : states_.eyes;
        resumeAfterWifi_ = nullptr;
        pendingAfterTransition_ = resume;
        enter(states_.transition);
    }
}

void StateMachine::enter(IAppState* state) {
    if (!state || current_ == state) return;
    if (current_) current_->onExit();
    current_ = state;
    current_->onEnter();
}

IAppState* StateMachine::peekNextContentState() {
    constexpr int kSlotCount = 7;
    for (int guard = 0; guard < kSlotCount; ++guard) {
        cycleIndex_ = (cycleIndex_ + 1) % kSlotCount;
        switch (cycleIndex_) {
            case 0: return states_.eyes;
            case 1: return states_.bitcoin;
            case 2: return states_.eyes;
            case 3: return states_.weather;
            case 4: return states_.eyes;
            case 5:
                if (rainForecast_ && rainForecast_()) return states_.rain;
                continue;  // sem previsão de chuva: pula para o próximo slot
            case 6: return states_.clock;
        }
    }
    return states_.eyes;
}

void StateMachine::goToNextInCycle() {
    pendingAfterTransition_ = peekNextContentState();
    enter(states_.transition);
}

void StateMachine::update(uint32_t dtMs, uint32_t nowMs) {
    // WiFi caiu: preempta na hora (urgente, sem transição holográfica).
    if (wifiDown_ && current_ != states_.wifiSearch) {
        resumeAfterWifi_ = current_;
        enter(states_.wifiSearch);
    }

    if (!current_) return;
    current_->update(dtMs, nowMs);
    if (!current_->finished()) return;

    if (current_ == states_.wifiSearch) return;  // aguarda notifyWifiUp()

    if (current_ == states_.boot) {
        cycleIndex_ = 0;  // primeiro conteúdo do ciclo já é Eyes
        enter(states_.eyes);
        return;
    }

    if (current_ == states_.transition) {
        enter(pendingAfterTransition_);
        pendingAfterTransition_ = nullptr;
        return;
    }

    goToNextInCycle();
}

void StateMachine::render(DisplayManager& display) {
    if (current_) current_->render(display);
}
