#pragma once
#include <cstdint>
#include "imu/QMI8658.h"

class DisplayManager;
class RobotFace;

// Liga o IMU ao robô: qualquer movimento acima do limiar aciona
// RobotFace::triggerWake() ("levantou o dispositivo -> acorda, olha pro
// usuário"). Brilho do display fica sempre no máximo (sem economia de
// energia por enquanto, a pedido do usuário).
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
};
