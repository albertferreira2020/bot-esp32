#include "screens/WifiSearchState.h"

#include <Arduino.h>

#include "config/BoardConfig.h"

void WifiSearchState::render(DisplayManager& display) {
    auto& canvas = display.canvas();
    canvas.fillScreen(TFT_BLACK);

    face_.render(display);

    uint32_t now = millis();
    float phase = (now % 2000) / 2000.0f;
    int cx = DisplayManager::width() / 2;
    int cy = 195;

    for (int i = 0; i < 3; ++i) {
        float t = phase - i * 0.28f;
        while (t < 0.0f) t += 1.0f;
        int radius = 8 + (int)(t * 34.0f);
        float fade = 1.0f - t;

        uint32_t base = board::robot::EYE_COLOR_DEFAULT;
        uint8_t r = (uint8_t)(((base >> 16) & 0xFF) * fade);
        uint8_t g = (uint8_t)(((base >> 8) & 0xFF) * fade);
        uint8_t b = (uint8_t)((base & 0xFF) * fade);
        uint32_t color = ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;

        canvas.drawArc(cx, cy, radius, radius + 2, 200.0f, 340.0f, display.tint(color));
    }

    canvas.setTextDatum(textdatum_t::middle_center);
    canvas.setTextColor(display.tint((uint32_t)0x999999));
    canvas.setTextSize(1.6f);
    canvas.drawString("Procurando sinal...", cx, 225);

    display.pushFull();
}
