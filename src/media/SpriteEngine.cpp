#include "media/SpriteEngine.h"

#include "animation/HoloFX.h"

namespace SpriteEngine {

void draw(DisplayManager& display, const SpriteFrame& frame, int cx, int cy, float scale,
          float rotationDeg, uint16_t transparentColor) {
    if (!frame.pixels || frame.w <= 0 || frame.h <= 0) return;
    display.canvas().pushImageRotateZoom(cx, cy, frame.w / 2.0f, frame.h / 2.0f, rotationDeg, scale,
                                          scale, frame.w, frame.h, frame.pixels, transparentColor);
}

void drawWithGlow(DisplayManager& display, const SpriteFrame& frame, int cx, int cy, float scale,
                  float rotationDeg, uint32_t glowColorRgb888, float glowIntensity,
                  uint16_t transparentColor) {
    if (glowIntensity > 0.01f) {
        int radius = (int)(0.5f * scale * ((frame.w + frame.h) / 2.0f) * 1.4f);
        holo::drawHalo(display, cx, cy, radius, glowColorRgb888, glowIntensity);
    }
    draw(display, frame, cx, cy, scale, rotationDeg, transparentColor);
}

}  // namespace SpriteEngine
