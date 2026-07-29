#include "screens/BitcoinState.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>

#include "animation/HoloFX.h"
#include "config/BoardConfig.h"

namespace {
constexpr uint32_t kBitcoinOrange = 0xF7931A;
constexpr uint32_t kRiseDurationMs = 1400;
constexpr uint32_t kSpinDurationMs = 2400;

// "US$ 118.432" — separador de milhar '.', sem centavos (texto maior/mais
// legível sem estourar a largura da tela em 240px).
void formatMoneyUSD(double value, char* out, size_t outSize) {
    long long whole = llround(value);
    char wholeBuf[32];
    snprintf(wholeBuf, sizeof(wholeBuf), "%lld", whole);
    int len = (int)strlen(wholeBuf);

    char grouped[40];
    int gi = 0;
    for (int i = 0; i < len; ++i) {
        if (i > 0 && (len - i) % 3 == 0) grouped[gi++] = '.';
        grouped[gi++] = wholeBuf[i];
    }
    grouped[gi] = '\0';
    snprintf(out, outSize, "US$ %s", grouped);
}
}  // namespace

void BitcoinState::onEnter() {
    elapsedMs_ = 0;
    spinAngle_ = 0.0f;
    riseY_.start(300.0f, 95.0f, kRiseDurationMs, EasingType::Bounce);
}

void BitcoinState::update(uint32_t dtMs, uint32_t nowMs) {
    (void)nowMs;
    elapsedMs_ += dtMs;
    riseY_.update(dtMs);
    if (riseY_.finished() && elapsedMs_ < kRiseDurationMs + kSpinDurationMs) {
        spinAngle_ += dtMs * 0.28f;
    }
}

void BitcoinState::render(DisplayManager& display) {
    auto& canvas = display.canvas();
    canvas.fillScreen(TFT_BLACK);

    int cx = DisplayManager::width() / 2;
    int cy = (int)riseY_.value();

    float rad = spinAngle_ * 3.14159265f / 180.0f;
    float squashX = std::fabs(std::cos(rad));
    constexpr int kBaseRadius = 46;
    int rx = std::max(6, (int)(kBaseRadius * squashX));
    int ry = kBaseRadius;

    holo::drawHalo(display, cx, cy, (int)(kBaseRadius * 1.6f), kBitcoinOrange, 0.45f);
    canvas.fillEllipse(cx, cy, rx, ry, display.tint(kBitcoinOrange));

    if (rx > kBaseRadius * 0.4f) {
        canvas.setTextDatum(textdatum_t::middle_center);
        canvas.setTextSize(3);
        canvas.setTextColor(display.tint((uint32_t)0xFFFFFF));
        canvas.drawString("B", cx, cy - 2);
        canvas.drawFastVLine(cx - 7, cy - ry - 3, 6, display.tint((uint32_t)0xFFFFFF));
        canvas.drawFastVLine(cx + 7, cy - ry - 3, 6, display.tint((uint32_t)0xFFFFFF));
    }

    if (elapsedMs_ >= kRiseDurationMs + kSpinDurationMs) {
        BitcoinSnapshot snap = scheduler_.bitcoin();
        canvas.setTextDatum(textdatum_t::middle_center);
        canvas.setTextSize(2.7f);
        canvas.setTextColor(display.tint((uint32_t)0xFFFFFF));

        if (snap.valid) {
            char priceBuf[48];
            formatMoneyUSD(snap.price, priceBuf, sizeof(priceBuf));
            canvas.drawString(priceBuf, cx, cy + 78);

            bool up = snap.changePercent >= 0.0;
            uint32_t arrowColor = up ? board::robot::EYE_COLORS_ALT[1] : board::robot::EYE_COLORS_ALT[3];
            char pctBuf[16];
            snprintf(pctBuf, sizeof(pctBuf), "%.2f%%", std::fabs(snap.changePercent));

            canvas.setTextSize(1.8f);
            canvas.setTextColor(display.tint(arrowColor));
            canvas.drawString(pctBuf, cx + 16, cy + 108);

            int tx = cx - 28, ty = cy + 108;
            if (up) {
                canvas.fillTriangle(tx - 6, ty + 5, tx + 6, ty + 5, tx, ty - 6, display.tint(arrowColor));
            } else {
                canvas.fillTriangle(tx - 6, ty - 5, tx + 6, ty - 5, tx, ty + 6, display.tint(arrowColor));
            }
        } else {
            canvas.drawString("carregando...", cx, cy + 80);
        }
    }

    display.pushFull();
}

bool BitcoinState::finished() const { return elapsedMs_ >= board::timing::SCREEN_DURATION_MS; }
