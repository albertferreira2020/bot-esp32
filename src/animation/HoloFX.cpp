#include "animation/HoloFX.h"

#include <Arduino.h>
#include <cmath>
#include <cstring>

namespace holo {

namespace {
uint32_t scaleColor(uint32_t rgb888, float scale) {
    if (scale < 0.0f) scale = 0.0f;
    if (scale > 1.0f) scale = 1.0f;
    uint8_t r = (uint8_t)(((rgb888 >> 16) & 0xFF) * scale);
    uint8_t g = (uint8_t)(((rgb888 >> 8) & 0xFF) * scale);
    uint8_t b = (uint8_t)((rgb888 & 0xFF) * scale);
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}
float randf(float lo, float hi) { return lo + (hi - lo) * ((float)random(0, 10000) / 10000.0f); }
}  // namespace

// ---------------------------------------------------------------------------
// ParticleField
// ---------------------------------------------------------------------------
void ParticleField::begin(int count, int originX, int originY, int radius) {
    count_ = count > kMax ? kMax : count;
    originX_ = originX;
    originY_ = originY;
    radius_ = radius;
    for (int i = 0; i < count_; ++i) respawn(particles_[i]);
}

void ParticleField::respawn(Particle& p) {
    float angle = randf(0.0f, 2.0f * (float)PI);
    float dist = randf(0.15f, 1.0f) * radius_;
    p.x = originX_ + std::cos(angle) * dist;
    p.y = originY_ + std::sin(angle) * dist;
    p.vx = randf(-6.0f, 6.0f) / 1000.0f;
    p.vy = randf(-10.0f, -2.0f) / 1000.0f;  // deriva sutil para cima
    p.maxLife = randf(2500.0f, 5000.0f);
    p.life = p.maxLife;
    p.size = randf(0.6f, 1.6f);
}

void ParticleField::update(uint32_t dtMs) {
    for (int i = 0; i < count_; ++i) {
        Particle& p = particles_[i];
        p.x += p.vx * dtMs;
        p.y += p.vy * dtMs;
        p.life -= dtMs;
        float dx = p.x - originX_, dy = p.y - originY_;
        bool outOfRange = (dx * dx + dy * dy) > (float)(radius_ * radius_ * 1.4f);
        if (p.life <= 0.0f || outOfRange) respawn(p);
    }
}

void ParticleField::render(DisplayManager& display, uint32_t colorRgb888) const {
    auto& canvas = display.canvas();
    for (int i = 0; i < count_; ++i) {
        const Particle& p = particles_[i];
        float lifeRatio = p.life / p.maxLife;
        float fade = lifeRatio < 0.5f ? lifeRatio * 2.0f : (1.0f - lifeRatio) * 2.0f;
        uint16_t color = display.tint((uint32_t)scaleColor(colorRgb888, 0.25f + 0.6f * fade));
        canvas.fillCircle((int)p.x, (int)p.y, (int)ceilf(p.size), color);
    }
}

// ---------------------------------------------------------------------------
// Halo
// ---------------------------------------------------------------------------
void drawHalo(DisplayManager& display, int cx, int cy, int radius, uint32_t colorRgb888,
              float intensity) {
    auto& canvas = display.canvas();
    int step = radius > 24 ? 3 : 2;
    for (int r = radius; r >= 1; r -= step) {
        float edgeT = (float)r / (float)radius;      // 1 na borda, 0 no centro
        float brightness = intensity * (1.0f - edgeT * 0.85f);
        uint16_t color = display.tint(scaleColor(colorRgb888, brightness));
        canvas.fillCircle(cx, cy, r, color);
    }
}

// ---------------------------------------------------------------------------
// Scanlines
// ---------------------------------------------------------------------------
void drawScanlines(DisplayManager& display, int x, int y, int w, int h, uint32_t phaseMs,
                    uint32_t colorRgb888, float intensity) {
    auto& canvas = display.canvas();
    int offset = (int)((phaseMs / 60) % 4);
    uint16_t color = display.tint(scaleColor(colorRgb888, intensity));
    for (int row = offset; row < h; row += 4) {
        canvas.drawFastHLine(x, y + row, w, color);
    }
}

// ---------------------------------------------------------------------------
// Glitch (desloca linhas horizontais reais no back-buffer)
// ---------------------------------------------------------------------------
void applyGlitch(DisplayManager& display, int x, int y, int w, int h, float intensity) {
    if (w <= 4 || h <= 0) return;
    auto* buf = static_cast<uint16_t*>(display.canvas().getBuffer());
    if (!buf) return;

    int screenW = DisplayManager::width();
    int bands = 1 + (int)(intensity * 5.0f);
    for (int i = 0; i < bands; ++i) {
        int ry = y + random(0, h);
        int maxShift = w / 4 + 1;
        int dx = random(-maxShift, maxShift + 1);
        if (dx == 0 || ry < 0 || ry >= DisplayManager::height()) continue;

        uint16_t* row = buf + (size_t)ry * screenW + x;
        if (dx > 0) {
            int keep = w - dx;
            if (keep > 0) memmove(row + dx, row, (size_t)keep * sizeof(uint16_t));
            for (int k = 0; k < dx && k < w; ++k) row[k] = 0;
        } else {
            int shift = -dx;
            int keep = w - shift;
            if (keep > 0) memmove(row, row + shift, (size_t)keep * sizeof(uint16_t));
            for (int k = keep < 0 ? 0 : keep; k < w; ++k) row[k] = 0;
        }
    }
}

}  // namespace holo
