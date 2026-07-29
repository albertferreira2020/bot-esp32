#pragma once
#include <cstdint>
#include "animation/HoloFX.h"
#include "animation/Tween.h"
#include "robot/EyeRenderer.h"
#include "robot/MoodTypes.h"

// O "coração" do robô: nunca fica parado. Controla humor, olhar, piscar,
// respiração, cor ocasional e os detalhes holográficos (partículas/scan) dos
// dois olhos. Quem usa (EyesState, WifiSearchState, etc.) só chama
// update()/render() e usa dirtyRect() para saber o que fazer push.
class RobotFace {
   public:
    void begin(int centerX, int centerY, int eyeSpacing, float eyeRadius);
    void update(uint32_t dtMs, uint32_t nowMs);
    void render(DisplayManager& display);

    Rect dirtyRect() const { return dirty_; }
    Mood mood() const { return mood_; }

    // Gatilhos externos (IMU / economia de energia).
    void triggerWake();
    void setSleepy(bool sleepy);

   private:
    void pickNewMood(uint32_t now);
    void updateBlink(uint32_t dtMs, uint32_t now);
    void updateBreathing(uint32_t dtMs);
    void updateScan(uint32_t dtMs, uint32_t now);
    uint32_t currentDisplayColor(uint32_t now) const;
    EyeParams computeEye(bool leftEye) const;

    int centerX_ = 120, centerY_ = 120, spacing_ = 90;
    float baseRadius_ = 34.0f;

    Mood mood_ = Mood::Idle;
    uint32_t moodUntil_ = 0;
    bool forcedSleepy_ = false;

    Tween gazeX_, gazeY_;
    Tween pupilScaleTween_;
    Tween eyelidMoodTween_;
    Tween eyelidBlinkTween_;
    Tween breatheTween_;
    bool breatheGrowing_ = true;

    bool blinking_ = false;
    uint32_t nextBlinkAt_ = 0;

    uint32_t colorFrom_ = 0xFFD800;
    uint32_t targetColor_ = 0xFFD800;
    uint32_t colorTransitionStart_ = 0;
    uint32_t colorTransitionDur_ = 1500;

    float scanPhase_ = -1.0f;
    uint32_t scanStart_ = 0;
    uint32_t scanDuration_ = 450;
    uint32_t nextScanAt_ = 0;

    holo::ParticleField particlesLeft_, particlesRight_;
    uint32_t lastNow_ = 0;
    Rect dirty_;
};
