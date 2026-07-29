#pragma once
#include <cstdint>
#include "display/DisplayManager.h"

// Camada comum de desenho de sprites RGB565 (PNG pré-convertido, spritesheet,
// frame de gif "congelado", ícone) com escala/rotação/fade/glow/blend. Usada
// hoje só como utilitário pronto para quando ícones (data/icons/) forem
// adicionados — os olhos e o logo do Bitcoin são desenhados proceduralmente
// (ver EyeRenderer e BitcoinState) para o firmware funcionar sem nenhum asset.
struct SpriteFrame {
    const uint16_t* pixels = nullptr;  // RGB565, w*h, packed
    int w = 0;
    int h = 0;
};

namespace SpriteEngine {

// Desenha `frame` centrado em (cx,cy). `scale` e `rotationDeg` dão a
// impressão de profundidade 3D (leve inclinação/zoom variável) pedida para
// elementos como o logo do Bitcoin. `transparentColor` marca o pixel
// "vazado" (alpha simples via color-key, sem custo de blend por pixel).
void draw(DisplayManager& display, const SpriteFrame& frame, int cx, int cy, float scale = 1.0f,
          float rotationDeg = 0.0f, uint16_t transparentColor = 0x0000);

// Halo atrás do sprite, reaproveitando holo::drawHalo.
void drawWithGlow(DisplayManager& display, const SpriteFrame& frame, int cx, int cy, float scale,
                  float rotationDeg, uint32_t glowColorRgb888, float glowIntensity,
                  uint16_t transparentColor = 0x0000);

}  // namespace SpriteEngine
