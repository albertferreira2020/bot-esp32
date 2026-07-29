#include "imu/QMI8658.h"

#include <Arduino.h>
#include <Wire.h>

#include "config/BoardConfig.h"

// Mapa de registradores do QMI8658 (comum às implementações de referência
// públicas do chip). Se o seu exemplar tiver silício/revisão diferente e a
// leitura não bater, ajuste só aqui.
namespace {
constexpr uint8_t REG_WHO_AM_I = 0x00;
constexpr uint8_t WHO_AM_I_VALUE = 0x05;
constexpr uint8_t REG_CTRL1 = 0x02;
constexpr uint8_t REG_CTRL2 = 0x03;  // accel: FS/ODR
constexpr uint8_t REG_CTRL3 = 0x04;  // gyro: FS/ODR
constexpr uint8_t REG_CTRL7 = 0x08;  // enable accel/gyro
constexpr uint8_t REG_AX_L = 0x35;   // 12 bytes: ax,ay,az,gx,gy,gz (LE, 16-bit)

// FS=±8g / ODR~250Hz para o accel; FS=±512dps / ODR~250Hz para o gyro.
constexpr uint8_t CTRL2_ACCEL_CONFIG = 0x23;
constexpr uint8_t CTRL3_GYRO_CONFIG = 0x53;
constexpr float ACCEL_SCALE_G_PER_LSB = 8.0f / 32768.0f;
constexpr float GYRO_SCALE_DPS_PER_LSB = 512.0f / 32768.0f;
}  // namespace

bool QMI8658::writeReg(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(board::IMU_I2C_ADDR);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool QMI8658::readRegs(uint8_t reg, uint8_t* buf, size_t len) {
    Wire.beginTransmission(board::IMU_I2C_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    size_t got = Wire.requestFrom((int)board::IMU_I2C_ADDR, (int)len);
    if (got != len) return false;
    for (size_t i = 0; i < len; ++i) buf[i] = Wire.read();
    return true;
}

bool QMI8658::begin() {
    Wire.begin(board::IMU_PIN_SDA, board::IMU_PIN_SCL, board::IMU_I2C_HZ);

    uint8_t who = 0;
    if (!readRegs(REG_WHO_AM_I, &who, 1) || who != WHO_AM_I_VALUE) {
        detected_ = false;
        return false;
    }

    writeReg(REG_CTRL1, 0x60);  // auto-increment de endereço
    writeReg(REG_CTRL2, CTRL2_ACCEL_CONFIG);
    writeReg(REG_CTRL3, CTRL3_GYRO_CONFIG);
    writeReg(REG_CTRL7, 0x03);  // aEN + gEN

    detected_ = true;
    return true;
}

bool QMI8658::read(ImuSample& out) {
    if (!detected_) return false;
    uint8_t raw[12];
    if (!readRegs(REG_AX_L, raw, sizeof(raw))) return false;

    auto toI16 = [&](int i) -> int16_t { return (int16_t)((raw[i + 1] << 8) | raw[i]); };

    out.ax = toI16(0) * ACCEL_SCALE_G_PER_LSB;
    out.ay = toI16(2) * ACCEL_SCALE_G_PER_LSB;
    out.az = toI16(4) * ACCEL_SCALE_G_PER_LSB;
    out.gx = toI16(6) * GYRO_SCALE_DPS_PER_LSB;
    out.gy = toI16(8) * GYRO_SCALE_DPS_PER_LSB;
    out.gz = toI16(10) * GYRO_SCALE_DPS_PER_LSB;
    return true;
}
