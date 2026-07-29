#include "screens/TransitionState.h"

#include <Arduino.h>
#include <algorithm>
#include <cmath>

#include "animation/Easing.h"
#include "animation/HoloFX.h"
#include "config/BoardConfig.h"

void TransitionState::onEnter() {
    elapsedMs_ = 0;
    usingVideo_ = mjpeg_.begin(display_, "/videos/transition");
    nextGlitchAt_ = millis() + random(150, 500);
    display_.canvas().fillScreen(TFT_BLACK);
    display_.pushFull();
}

void TransitionState::update(uint32_t dtMs, uint32_t nowMs) {
    elapsedMs_ += dtMs;
    if (usingVideo_) mjpeg_.update(nowMs);
}

void TransitionState::render(DisplayManager& display) {
    if (usingVideo_ && mjpeg_.isOpen()) {
        display.pushRegion(0, 0, DisplayManager::width(), DisplayManager::height());
        return;
    }
    usingVideo_ = false;

    int w = DisplayManager::width(), h = DisplayManager::height();
    auto& canvas = display.canvas();
    canvas.fillScreen(TFT_BLACK);

    float t = std::min(1.0f, (float)elapsedMs_ / (float)board::timing::TRANSITION_DURATION_MS);
    // Intensidade sobe, pico no meio, cai no fim (fade in -> fade out do próprio efeito).
    constexpr float kPi = 3.14159265358979323846f;
    float envelope = std::sin(t * kPi);

    holo::drawScanlines(display, 0, 0, w, h, millis(), 0x2A3A40, 0.25f + 0.2f * envelope);
    holo::drawHalo(display, w / 2, h / 2, (int)(30 + envelope * 70), 0x36FFF0, 0.3f + 0.4f * envelope);

    uint32_t now = millis();
    if (now >= nextGlitchAt_) {
        holo::applyGlitch(display, 0, 0, w, h, 0.4f + 0.5f * envelope);
        nextGlitchAt_ = now + random(120, 420);
    }

    display.pushFull();
}

bool TransitionState::finished() const {
    return elapsedMs_ >= board::timing::TRANSITION_DURATION_MS;
}
