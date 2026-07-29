#pragma once
#include <cstdint>
#include "display/DisplayManager.h"

// Parâmetros já resolvidos de um olho num dado frame — o RobotFace calcula
// isso a partir do humor/animações; o EyeRenderer só sabe desenhar.
struct EyeParams {
    int cx = 0, cy = 0;
    float baseRadius = 34.0f;
    float scale = 1.0f;        // respiração + humor
    float eyelid = 1.0f;       // 0 fechado (piscar) .. 1 aberto
    float pupilOffsetX = 0.0f;  // -1..1
    float pupilOffsetY = 0.0f;  // -1..1
    float pupilScale = 1.0f;    // <1 assustado/contraído, >1 dilatado
    uint32_t colorRgb888 = 0xFFD800;
    float glow = 0.6f;          // intensidade do halo, 0..1
    float scanPhase = -1.0f;    // -1 = sem escaneamento; senão 0..1 progresso
};

namespace EyeRenderer {

// Desenha um olho no canvas. Não faz push — quem chama decide a região suja.
void draw(DisplayManager& display, const EyeParams& p);

// Bounding box (com folga p/ halo) do olho, útil para dirty-rect.
Rect bounds(const EyeParams& p);

}  // namespace EyeRenderer
