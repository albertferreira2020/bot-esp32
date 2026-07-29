#include "screens/BootState.h"

#include <algorithm>
#include <Arduino.h>

#include "animation/Easing.h"
#include "animation/HoloFX.h"
#include "config/BoardConfig.h"

void BootState::onEnter() {
    elapsedMs_ = 0;
    usingVideo_ = mjpeg_.begin(display_, "/videos/boot");
    display_.canvas().fillScreen(TFT_BLACK);
    display_.pushFull();
}

void BootState::update(uint32_t dtMs, uint32_t nowMs) {
    elapsedMs_ += dtMs;
    if (usingVideo_) mjpeg_.update(nowMs);
}

void BootState::render(DisplayManager& display) {
    if (usingVideo_ && mjpeg_.isOpen()) {
        display.pushRegion(0, 0, DisplayManager::width(), DisplayManager::height());
        return;
    }
    usingVideo_ = false;

    auto& canvas = display.canvas();
    canvas.fillScreen(TFT_BLACK);

    float t = std::min(1.0f, (float)elapsedMs_ / (float)board::timing::BOOT_DURATION_MS);
    float pulse = ease(EasingType::EaseInOut, t);

    holo::drawScanlines(display, 0, 0, DisplayManager::width(), DisplayManager::height(), millis(),
                        0x1A2A30, 0.18f);
    holo::drawHalo(display, DisplayManager::width() / 2, DisplayManager::height() / 2,
                   (int)(18 + pulse * 95), board::robot::EYE_COLOR_DEFAULT, 0.4f + 0.35f * pulse);

    if (t > 0.55f) {
        holo::applyGlitch(display, 0, 0, DisplayManager::width(), DisplayManager::height(),
                          (1.0f - t) * 0.7f);
    }

    display.pushFull();
}

bool BootState::finished() const { return elapsedMs_ >= board::timing::BOOT_DURATION_MS; }
