#pragma once
#include <cstdint>
#include "display/DisplayManager.h"

// Efeitos hologáficos procedurais — sem depender de nenhum vídeo/gif: glitch,
// scanlines, partículas e halo. Usados na transição e como detalhes sutis
// nas demais telas para valorizar o prisma sobre fundo 100% preto.
namespace holo {

// Pequenas partículas flutuando ao redor de um ponto (ex.: perto dos olhos).
class ParticleField {
   public:
    void begin(int count, int originX, int originY, int radius);
    void update(uint32_t dtMs);
    void render(DisplayManager& display, uint32_t colorRgb888) const;

   private:
    struct Particle {
        float x, y, vx, vy, life, maxLife, size;
    };
    static constexpr int kMax = 24;
    Particle particles_[kMax]{};
    int count_ = 0;
    int originX_ = 0, originY_ = 0, radius_ = 0;
    void respawn(Particle& p);
};

// Halo luminoso discreto (glow radial por círculos concêntricos).
void drawHalo(DisplayManager& display, int cx, int cy, int radius, uint32_t colorRgb888,
              float intensity);

// Linhas de varredura sutis, animadas, sobre uma região.
void drawScanlines(DisplayManager& display, int x, int y, int w, int h, uint32_t phaseMs,
                    uint32_t colorRgb888 = 0x2A3A40, float intensity = 0.35f);

// Glitch digital: desloca horizontalmente faixas aleatórias já desenhadas na
// região (chamar depois do conteúdo estar no canvas; some com display.pushRegion
// depois para exibir).
void applyGlitch(DisplayManager& display, int x, int y, int w, int h, float intensity);

}  // namespace holo
