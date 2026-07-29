#include "robot/EyeRenderer.h"

#include <algorithm>
#include "animation/HoloFX.h"

namespace EyeRenderer {

void draw(DisplayManager& display, const EyeParams& p) {
    auto& canvas = display.canvas();

    float rx = p.baseRadius * p.scale;
    float ry = std::max(1.0f, rx * p.eyelid);

    if (p.glow > 0.01f) {
        holo::drawHalo(display, p.cx, p.cy, (int)(rx * 1.7f), p.colorRgb888, p.glow * 0.5f);
    }

    canvas.fillEllipse(p.cx, p.cy, (int)rx, (int)ry, display.tint(p.colorRgb888));

    // Pupila + reflexo só aparecem com o olho suficientemente aberto.
    if (p.eyelid > 0.15f) {
        float pupilR = rx * 0.42f * p.pupilScale;
        float maxOffX = std::max(0.0f, rx - pupilR) * 0.8f;
        float maxOffY = std::max(0.0f, ry - pupilR) * 0.8f;
        int px = p.cx + (int)(p.pupilOffsetX * maxOffX);
        int py = p.cy + (int)(p.pupilOffsetY * maxOffY);
        float pupilRy = std::min(pupilR, ry * 0.9f);

        canvas.fillEllipse(px, py, (int)pupilR, (int)pupilRy, display.tint((uint32_t)0x101010));

        int glintR = (int)(pupilR * 0.28f) + 1;
        canvas.fillCircle(px - (int)(pupilR * 0.35f), py - (int)(pupilR * 0.35f), glintR,
                           display.tint((uint32_t)0xFFFFFF));

        // Reflexo suave percorrendo a íris (arco claro na borda superior).
        uint8_t hr = (uint8_t)std::min<int>(255, ((p.colorRgb888 >> 16) & 0xFF) + 90);
        uint8_t hg = (uint8_t)std::min<int>(255, ((p.colorRgb888 >> 8) & 0xFF) + 90);
        uint8_t hb = (uint8_t)std::min<int>(255, (p.colorRgb888 & 0xFF) + 90);
        uint32_t highlightColor = ((uint32_t)hr << 16) | ((uint32_t)hg << 8) | hb;
        canvas.fillArc(p.cx, p.cy, (int)(rx * 0.86f), (int)(rx * 0.98f), 200.0f, 260.0f,
                        display.tint(highlightColor));

        // Efeito de "escaneamento" ocasional na pupila.
        if (p.scanPhase >= 0.0f && p.scanPhase <= 1.0f) {
            int scanY = py - (int)pupilRy + (int)(p.scanPhase * 2.0f * pupilRy);
            canvas.drawFastHLine(px - (int)pupilR, scanY, (int)(pupilR * 2),
                                 display.tint((uint32_t)0x8CFFFA));
        }
    }
}

Rect bounds(const EyeParams& p) {
    float rx = p.baseRadius * p.scale * 1.8f;  // folga p/ halo
    int x = (int)(p.cx - rx);
    int y = (int)(p.cy - rx);
    int size = (int)(rx * 2.0f);
    return Rect{x, y, size, size};
}

}  // namespace EyeRenderer
