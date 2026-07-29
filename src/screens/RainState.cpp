#include "screens/RainState.h"

#include <Arduino.h>
#include <cstdio>

#include "config/BoardConfig.h"
#include "network/TimeManager.h"

namespace {
constexpr uint32_t kCardAppearAtMs = 3500;
}

void RainState::onEnter() {
    elapsedMs_ = 0;
    usingVideo_ = mjpeg_.begin(display_, "/videos/rain");
    for (auto& d : drops_) {
        d.x = (float)random(0, DisplayManager::width());
        d.y = (float)random(0, DisplayManager::height());
        d.speed = 120.0f + (float)random(0, 160);
        d.length = 6.0f + (float)random(0, 10);
    }
}

void RainState::update(uint32_t dtMs, uint32_t nowMs) {
    elapsedMs_ += dtMs;
    if (usingVideo_) {
        mjpeg_.update(nowMs);
        return;
    }
    float dt = dtMs / 1000.0f;
    for (auto& d : drops_) {
        d.y += d.speed * dt;
        if (d.y - d.length > DisplayManager::height()) {
            d.y = -d.length;
            d.x = (float)random(0, DisplayManager::width());
        }
    }
}

void RainState::render(DisplayManager& display) {
    bool videoActive = usingVideo_ && mjpeg_.isOpen();
    auto& canvas = display.canvas();

    if (!videoActive) {
        usingVideo_ = false;
        canvas.fillScreen(TFT_BLACK);
        uint16_t dropColor = display.tint((uint32_t)0x4E8CB0);
        for (const auto& d : drops_) {
            canvas.drawFastVLine((int)d.x, (int)d.y, (int)d.length, dropColor);
        }
    }
    // Se videoActive, o frame já foi decodificado direto no canvas por
    // mjpeg_.update() (callback do TJpg_Decoder).

    if (elapsedMs_ >= kCardAppearAtMs) {
        int cx = DisplayManager::width() / 2;
        int cardY = 150, cardH = 80;
        canvas.fillRoundRect(cx - 90, cardY, 180, cardH, 10, display.tint((uint32_t)0x0A1A22));

        WeatherSnapshot snap = scheduler_.weather();
        TimeManager::Now now = TimeManager::now();
        int nextHour = (now.hour + 1) % 24;

        canvas.setTextDatum(textdatum_t::middle_center);
        canvas.setTextColor(display.tint(board::robot::EYE_COLORS_ALT[2]));
        canvas.setTextSize(2);
        canvas.drawString("Vai chover", cx, cardY + 22);

        char timeBuf[8], pctBuf[8];
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", nextHour, now.minute);
        snprintf(pctBuf, sizeof(pctBuf), "%d%%", snap.valid ? snap.rainProbabilityPct : 0);

        canvas.setTextColor(display.tint((uint32_t)0xFFFFFF));
        canvas.setTextSize(1.8f);
        canvas.drawString(timeBuf, cx - 40, cardY + 55);
        canvas.drawString(pctBuf, cx + 40, cardY + 55);
    }

    display.pushFull();
}

bool RainState::finished() const { return elapsedMs_ >= board::timing::SCREEN_DURATION_MS; }
