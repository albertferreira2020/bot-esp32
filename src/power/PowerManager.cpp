#include "power/PowerManager.h"

#include <Arduino.h>
#include <cmath>

#include "config/BoardConfig.h"
#include "display/DisplayManager.h"
#include "robot/RobotFace.h"

bool PowerManager::begin() {
    imuOk_ = imu_.begin();
    lastMotionAt_ = millis();
    return imuOk_;
}

void PowerManager::update(uint32_t nowMs, DisplayManager& display, RobotFace& face) {
    if (imuOk_) {
        ImuSample sample;
        if (imu_.read(sample)) {
            float magnitude = std::sqrt(sample.ax * sample.ax + sample.ay * sample.ay + sample.az * sample.az);
            if (hasSample_) {
                float delta = std::fabs(magnitude - lastMagnitude_);
                if (delta > board::power::MOTION_WAKE_THRESHOLD_G) {
                    lastMotionAt_ = nowMs;
                    if (dimmed_) {
                        dimmed_ = false;
                        display.setBrightness(1.0f);
                        face.setSleepy(false);
                    }
                    face.triggerWake();
                }
            }
            lastMagnitude_ = magnitude;
            hasSample_ = true;
        }
    }

    bool shouldDim = (nowMs - lastMotionAt_) > board::power::IDLE_TIMEOUT_MS;
    if (shouldDim && !dimmed_) {
        dimmed_ = true;
        display.setBrightness(1.0f - (float)board::power::DIM_OVERLAY_ALPHA / 255.0f);
        face.setSleepy(true);
    }
}
