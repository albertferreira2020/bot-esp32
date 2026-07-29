#include "screens/ClockState.h"

#include <Arduino.h>
#include <cstdio>

#include "animation/HoloFX.h"
#include "config/BoardConfig.h"
#include "network/TimeManager.h"

void ClockState::render(DisplayManager& display) {
    auto& canvas = display.canvas();
    canvas.fillScreen(TFT_BLACK);

    int cx = DisplayManager::width() / 2;
    uint32_t nowMs = millis();

    if (!TimeManager::synced()) {
        canvas.setTextDatum(textdatum_t::middle_center);
        canvas.setTextColor(display.tint((uint32_t)0xFFFFFF));
        canvas.setTextSize(2);
        canvas.drawString("sincronizando...", cx, 120);
        display.pushFull();
        return;
    }

    TimeManager::Now now = TimeManager::now();
    bool colonOn = (nowMs / 500) % 2 == 0;

    char hourBuf[4], minBuf[4];
    snprintf(hourBuf, sizeof(hourBuf), "%02d", now.hour);
    snprintf(minBuf, sizeof(minBuf), "%02d", now.minute);

    canvas.setTextDatum(textdatum_t::middle_center);
    canvas.setTextColor(display.tint(board::robot::EYE_COLOR_DEFAULT));
    canvas.setTextSize(6);
    canvas.drawString(hourBuf, cx - 46, 108);
    canvas.drawString(minBuf, cx + 46, 108);
    if (colonOn) canvas.drawString(":", cx, 108);

    canvas.setTextSize(2);
    canvas.setTextColor(display.tint((uint32_t)0xCCCCCC));
    canvas.drawString(TimeManager::weekdayName(now.weekday), cx, 168);

    char dateBuf[24];
    snprintf(dateBuf, sizeof(dateBuf), "%d %s", now.day, TimeManager::monthName(now.month));
    canvas.drawString(dateBuf, cx, 192);

    holo::drawScanlines(display, 0, 90, DisplayManager::width(), 40, nowMs, 0x1A2A30, 0.12f);

    display.pushFull();
}

bool ClockState::finished() const { return elapsedMs_ >= board::timing::SCREEN_DURATION_MS; }
