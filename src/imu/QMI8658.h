#pragma once
#include <cstdint>
#include <cstddef>

struct ImuSample {
    float ax = 0.0f, ay = 0.0f, az = 0.0f;  // g (aprox., FS=±8g)
    float gx = 0.0f, gy = 0.0f, gz = 0.0f;  // deg/s (aprox., FS=±512dps)
};

// Driver mínimo por registrador para o QMI8658 (I2C). Só o essencial: detecta
// o chip (WHO_AM_I), habilita accel+gyro e lê as 6 amostras. O PowerManager
// só precisa da variação relativa entre leituras (detectar movimento/parado),
// então a calibração exata de escala não é crítica aqui.
class QMI8658 {
   public:
    bool begin();
    bool read(ImuSample& out);
    bool detected() const { return detected_; }

   private:
    bool writeReg(uint8_t reg, uint8_t value);
    bool readRegs(uint8_t reg, uint8_t* buf, size_t len);

    bool detected_ = false;
};
