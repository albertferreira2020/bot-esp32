#include "robot/RobotFace.h"

#include <Arduino.h>
#include <algorithm>
#include <cmath>
#include "config/BoardConfig.h"

namespace {
float clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
}  // namespace

void RobotFace::begin(int centerX, int centerY, int eyeSpacing, float eyeRadius) {
    centerX_ = centerX;
    centerY_ = centerY;
    spacing_ = eyeSpacing;
    baseRadius_ = eyeRadius;

    gazeX_.snapTo(0.0f);
    gazeY_.snapTo(0.0f);
    pupilScaleTween_.snapTo(1.0f);
    eyelidMoodTween_.snapTo(1.0f);
    eyelidBlinkTween_.snapTo(1.0f);
    breatheTween_.start(1.0f, 0.97f, 2200, EasingType::EaseInOut);

    colorFrom_ = board::robot::EYE_COLOR_DEFAULT;
    targetColor_ = board::robot::EYE_COLOR_DEFAULT;

    nextBlinkAt_ = millis() + random(1000, 3000);
    nextScanAt_ = millis() + random(5000, 12000);

    particlesLeft_.begin(6, centerX_ - spacing_ / 2, centerY_, (int)(eyeRadius * 2.4f));
    particlesRight_.begin(6, centerX_ + spacing_ / 2, centerY_, (int)(eyeRadius * 2.4f));

    moodUntil_ = millis();
}

void RobotFace::pickNewMood(uint32_t now) {
    if (forcedSleepy_) {
        mood_ = Mood::Sleepy;
        moodUntil_ = now + 4000;
    } else {
        static const Mood kOptions[] = {
            Mood::Idle,   Mood::Idle,    Mood::Curious, Mood::LookLeft, Mood::LookRight,
            Mood::LookUp, Mood::Happy,   Mood::Smiling, Mood::LookUser, Mood::Scared,
        };
        mood_ = kOptions[random(0, (long)(sizeof(kOptions) / sizeof(kOptions[0])))];
        moodUntil_ = now + (uint32_t)random(2500, 6000);
    }

    float gx = 0.0f, gy = 0.0f, eyelidTarget = 1.0f, pupilTarget = 1.0f;
    switch (mood_) {
        case Mood::Idle: break;
        case Mood::Curious:
            gx = ((float)random(-100, 100)) / 200.0f;
            gy = ((float)random(-60, 20)) / 200.0f;
            pupilTarget = 1.2f;
            break;
        case Mood::Sleepy: eyelidTarget = 0.35f; gy = 0.25f; break;
        case Mood::Happy: eyelidTarget = 0.8f; gy = -0.15f; break;
        case Mood::Smiling: eyelidTarget = 0.6f; gy = -0.25f; break;
        case Mood::Scared: eyelidTarget = 1.15f; pupilTarget = 0.6f; break;
        case Mood::LookLeft: gx = -0.85f; break;
        case Mood::LookRight: gx = 0.85f; break;
        case Mood::LookUp: gy = -0.85f; break;
        case Mood::LookUser: pupilTarget = 1.1f; break;
    }

    gazeX_.start(gazeX_.value(), gx, 900, EasingType::Spring);
    gazeY_.start(gazeY_.value(), gy, 900, EasingType::Spring);
    pupilScaleTween_.start(pupilScaleTween_.value(), pupilTarget, 700, EasingType::EaseInOut);
    eyelidMoodTween_.start(eyelidMoodTween_.value(), eyelidTarget, 700, EasingType::EaseInOut);

    // Ocasionalmente muda a cor dos olhos (padrão: amarelo).
    float roll = (float)random(0, 10000) / 10000.0f;
    if (targetColor_ == board::robot::EYE_COLOR_DEFAULT &&
        roll < board::robot::MOOD_COLOR_SHIFT_CHANCE) {
        int idx = random(0, board::robot::EYE_COLORS_ALT_COUNT);
        colorFrom_ = currentDisplayColor(now);
        targetColor_ = board::robot::EYE_COLORS_ALT[idx];
        colorTransitionStart_ = now;
    } else if (targetColor_ != board::robot::EYE_COLOR_DEFAULT && roll < 0.35f) {
        colorFrom_ = currentDisplayColor(now);
        targetColor_ = board::robot::EYE_COLOR_DEFAULT;
        colorTransitionStart_ = now;
    }
}

void RobotFace::updateBreathing(uint32_t dtMs) {
    breatheTween_.update(dtMs);
    if (breatheTween_.finished()) {
        breatheGrowing_ = !breatheGrowing_;
        float from = breatheGrowing_ ? 0.97f : 1.0f;
        float to = breatheGrowing_ ? 1.0f : 0.97f;
        breatheTween_.start(from, to, 2200, EasingType::EaseInOut);
    }
}

void RobotFace::updateBlink(uint32_t dtMs, uint32_t now) {
    eyelidBlinkTween_.update(dtMs);
    if (!blinking_ && now >= nextBlinkAt_) {
        blinking_ = true;
        eyelidBlinkTween_.start(1.0f, 0.0f, 90, EasingType::EaseInOut);
    } else if (blinking_ && eyelidBlinkTween_.finished()) {
        if (eyelidBlinkTween_.value() < 0.5f) {
            eyelidBlinkTween_.start(0.0f, 1.0f, 120, EasingType::EaseInOut);
        } else {
            blinking_ = false;
            nextBlinkAt_ = now + (uint32_t)random(2000, 6000);
        }
    }
}

void RobotFace::updateScan(uint32_t dtMs, uint32_t now) {
    (void)dtMs;
    if (scanPhase_ < 0.0f) {
        if (now >= nextScanAt_) {
            scanPhase_ = 0.0f;
            scanStart_ = now;
        }
        return;
    }
    float t = (float)(now - scanStart_) / (float)scanDuration_;
    if (t >= 1.0f) {
        scanPhase_ = -1.0f;
        nextScanAt_ = now + (uint32_t)random(6000, 14000);
    } else {
        scanPhase_ = t;
    }
}

uint32_t RobotFace::currentDisplayColor(uint32_t now) const {
    if (now >= colorTransitionStart_ + colorTransitionDur_) return targetColor_;
    float t = clampf((float)(now - colorTransitionStart_) / (float)colorTransitionDur_, 0.0f, 1.0f);
    auto lerpCh = [&](int shift) -> uint8_t {
        uint8_t a = (colorFrom_ >> shift) & 0xFF;
        uint8_t b = (targetColor_ >> shift) & 0xFF;
        return (uint8_t)(a + (b - a) * t);
    };
    return ((uint32_t)lerpCh(16) << 16) | ((uint32_t)lerpCh(8) << 8) | lerpCh(0);
}

void RobotFace::update(uint32_t dtMs, uint32_t nowMs) {
    lastNow_ = nowMs;
    if (nowMs >= moodUntil_) pickNewMood(nowMs);

    gazeX_.update(dtMs);
    gazeY_.update(dtMs);
    pupilScaleTween_.update(dtMs);
    eyelidMoodTween_.update(dtMs);

    updateBreathing(dtMs);
    updateBlink(dtMs, nowMs);
    updateScan(dtMs, nowMs);

    particlesLeft_.update(dtMs);
    particlesRight_.update(dtMs);
}

EyeParams RobotFace::computeEye(bool leftEye) const {
    EyeParams p;
    p.cx = leftEye ? centerX_ - spacing_ / 2 : centerX_ + spacing_ / 2;
    p.cy = centerY_;
    p.baseRadius = baseRadius_;
    p.scale = breatheTween_.value();
    p.eyelid = eyelidMoodTween_.value() * eyelidBlinkTween_.value();

    float jitterPhase = leftEye ? 0.0f : 1.3f;
    float jitterX = std::sin(lastNow_ * 0.0021f + jitterPhase) * 0.05f +
                    std::sin(lastNow_ * 0.0037f + jitterPhase * 2.0f) * 0.03f;
    float jitterY = std::cos(lastNow_ * 0.0018f + jitterPhase) * 0.04f;

    p.pupilOffsetX = clampf(gazeX_.value() + jitterX, -1.0f, 1.0f);
    p.pupilOffsetY = clampf(gazeY_.value() + jitterY, -1.0f, 1.0f);
    p.pupilScale = pupilScaleTween_.value();
    p.colorRgb888 = currentDisplayColor(lastNow_);
    p.glow = 0.55f + 0.15f * std::sin(lastNow_ * 0.0017f);
    p.scanPhase = scanPhase_;
    return p;
}

void RobotFace::render(DisplayManager& display) {
    EyeParams left = computeEye(true);
    EyeParams right = computeEye(false);

    Rect lb = EyeRenderer::bounds(left);
    Rect rb = EyeRenderer::bounds(right);
    lb.x -= 20; lb.y -= 20; lb.w += 40; lb.h += 40;
    rb.x -= 20; rb.y -= 20; rb.w += 40; rb.h += 40;
    dirty_ = lb.united(rb);

    auto& canvas = display.canvas();
    canvas.fillRect(dirty_.x, dirty_.y, dirty_.w, dirty_.h, TFT_BLACK);

    particlesLeft_.render(display, left.colorRgb888);
    particlesRight_.render(display, right.colorRgb888);

    EyeRenderer::draw(display, left);
    EyeRenderer::draw(display, right);
}

void RobotFace::triggerWake() {
    uint32_t now = lastNow_;
    mood_ = Mood::LookUser;
    moodUntil_ = now + 2500;
    gazeX_.start(gazeX_.value(), 0.0f, 400, EasingType::Spring);
    gazeY_.start(gazeY_.value(), 0.0f, 400, EasingType::Spring);
    pupilScaleTween_.start(pupilScaleTween_.value(), 1.3f, 500, EasingType::Elastic);
    eyelidMoodTween_.start(eyelidMoodTween_.value(), 1.25f, 500, EasingType::Elastic);
}

void RobotFace::setSleepy(bool sleepy) {
    forcedSleepy_ = sleepy;
    if (sleepy && mood_ != Mood::Sleepy) {
        mood_ = Mood::Sleepy;
        moodUntil_ = lastNow_ + 999999;
        eyelidMoodTween_.start(eyelidMoodTween_.value(), 0.35f, 900, EasingType::EaseInOut);
        pupilScaleTween_.start(pupilScaleTween_.value(), 0.9f, 900, EasingType::EaseInOut);
        gazeX_.start(gazeX_.value(), 0.0f, 900, EasingType::EaseInOut);
        gazeY_.start(gazeY_.value(), 0.3f, 900, EasingType::EaseInOut);
    } else if (!sleepy && mood_ == Mood::Sleepy) {
        moodUntil_ = lastNow_;
    }
}
