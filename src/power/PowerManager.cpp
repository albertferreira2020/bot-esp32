#include "power/PowerManager.h"

#include <cmath>

#include "config/BoardConfig.h"
#include "display/DisplayManager.h"
#include "robot/RobotFace.h"

bool PowerManager::begin() {
    imuOk_ = imu_.begin();
    return imuOk_;
}

void PowerManager::update(uint32_t nowMs, DisplayManager& display, RobotFace& face) {
    (void)nowMs;
    display.setBrightness(1.0f);  // brilho sempre no máximo

    if (!imuOk_) return;

    ImuSample sample;
    if (!imu_.read(sample)) return;

    float magnitude = std::sqrt(sample.ax * sample.ax + sample.ay * sample.ay + sample.az * sample.az);
    if (hasSample_) {
        float delta = std::fabs(magnitude - lastMagnitude_);
        if (delta > board::power::MOTION_WAKE_THRESHOLD_G) face.triggerWake();
    }
    lastMagnitude_ = magnitude;
    hasSample_ = true;
}
