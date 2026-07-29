#include "screens/WeatherState.h"

#include <Arduino.h>
#include <cmath>
#include <cstdio>

#include "animation/HoloFX.h"
#include "config/BoardConfig.h"
#include "network/WeatherClient.h"

namespace {
void drawCloud(lgfx::LGFX_Sprite& canvas, int x, int y, int scale, uint16_t color) {
    canvas.fillCircle(x, y, 10 * scale / 10, color);
    canvas.fillCircle(x - 12 * scale / 10, y + 3 * scale / 10, 8 * scale / 10, color);
    canvas.fillCircle(x + 13 * scale / 10, y + 3 * scale / 10, 8 * scale / 10, color);
    canvas.fillCircle(x, y + 5 * scale / 10, 9 * scale / 10, color);
    canvas.fillRect(x - 14 * scale / 10, y, 28 * scale / 10, 8 * scale / 10, color);
}

void drawWeatherIcon(DisplayManager& display, int cx, int cy, int weatherCode, uint32_t nowMs) {
    auto& canvas = display.canvas();
    float driftA = std::sin(nowMs * 0.00035f) * 10.0f;
    float driftB = std::sin(nowMs * 0.00022f + 2.1f) * 7.0f;

    uint16_t sunColor = display.tint(board::robot::EYE_COLOR_DEFAULT);
    uint16_t cloudColorBack = display.tint((uint32_t)0x445A66);
    uint16_t cloudColorFront = display.tint((uint32_t)0x8FA6B0);

    bool showSun = weatherCode <= 1;
    if (showSun) {
        holo::drawHalo(display, cx - 20, cy - 18, 30, board::robot::EYE_COLOR_DEFAULT, 0.35f);
        canvas.fillCircle(cx - 20, cy - 18, 14, sunColor);
    }
    if (weatherCode >= 2) {
        drawCloud(canvas, (int)(cx + 8 + driftB), cy + 6, 12, cloudColorBack);
        drawCloud(canvas, (int)(cx - 6 + driftA), cy - 4, 16, cloudColorFront);
    }
}
}  // namespace

void WeatherState::render(DisplayManager& display) {
    auto& canvas = display.canvas();
    canvas.fillScreen(TFT_BLACK);

    WeatherSnapshot snap = scheduler_.weather();
    int cx = DisplayManager::width() / 2;

    drawWeatherIcon(display, cx, 78, snap.valid ? snap.weatherCode : 2, millis());

    canvas.setTextDatum(textdatum_t::middle_center);
    canvas.setTextColor(display.tint((uint32_t)0xFFFFFF));

    if (!snap.valid) {
        canvas.setTextSize(2);
        canvas.drawString("carregando...", cx, 130);
        display.pushFull();
        return;
    }

    char tempBuf[16];
    snprintf(tempBuf, sizeof(tempBuf), "%.0f", snap.tempC);
    canvas.setTextSize(4);
    canvas.drawString(tempBuf, cx - 10, 128);
    int degX = cx - 10 + (int)(canvas.textWidth(tempBuf) / 2) + 14;
    canvas.drawCircle(degX, 110, 4, display.tint((uint32_t)0xFFFFFF));
    canvas.setTextSize(2);
    canvas.drawString("C", degX + 14, 128);

    canvas.setTextSize(1.6f);
    canvas.setTextColor(display.tint(board::robot::EYE_COLOR_DEFAULT));
    canvas.drawString(weatherCodeDescription(snap.weatherCode), cx, 156);

    canvas.setTextSize(1.4f);
    canvas.setTextColor(display.tint((uint32_t)0xAAAAAA));
    canvas.drawString("Umidade", cx - 55, 182);
    canvas.drawString("Vento", cx + 55, 182);

    char humidityBuf[16], windBuf[16];
    snprintf(humidityBuf, sizeof(humidityBuf), "%d%%", snap.humidityPct);
    snprintf(windBuf, sizeof(windBuf), "%.0f km/h", snap.windKmh);

    canvas.setTextSize(1.8f);
    canvas.setTextColor(display.tint((uint32_t)0xFFFFFF));
    canvas.drawString(humidityBuf, cx - 55, 204);
    canvas.drawString(windBuf, cx + 55, 204);

    display.pushFull();
}

bool WeatherState::finished() const { return elapsedMs_ >= board::timing::SCREEN_DURATION_MS; }
