#pragma once
#include <cstdint>
#include "imu/QMI8658.h"

class DisplayManager;
class RobotFace;

// Liga o IMU à economia de energia: 3 min parado -> reduz brilho (overlay,
// ver DisplayManager::tint); qualquer movimento acima do limiar -> brilho
// máximo de novo + acorda o robô (RobotFace::triggerWake). Sem o chip
// detectado, funciona em modo degradado: ainda reduz brilho após o timeout
// (não tem como saber que "acordou" sem sensor).
class PowerManager {
   public:
    bool begin();
    void update(uint32_t nowMs, DisplayManager& display, RobotFace& face);

    bool imuDetected() const { return imuOk_; }

   private:
    QMI8658 imu_;
    bool imuOk_ = false;
    float lastMagnitude_ = 0.0f;
    bool hasSample_ = false;
    uint32_t lastMotionAt_ = 0;
    bool dimmed_ = false;
};
