#pragma once
#include <cstdint>

// ============================================================================
// Waveshare ESP32-S3-LCD-1.3 — pinout
// Confirmado via documentação pública da placa (display ST7789 + IMU QMI8658
// onboard). Se o seu exemplar divergir, ajuste só aqui.
// ============================================================================
namespace board {

// --- LCD (ST7789, SPI) ---
constexpr int LCD_PIN_DC   = 38;
constexpr int LCD_PIN_CS   = 39;
constexpr int LCD_PIN_SCLK = 40;
constexpr int LCD_PIN_MOSI = 41;
constexpr int LCD_PIN_RST  = 42;
// Nenhuma fonte confirma um GPIO de backlight dedicado nesta placa (BL fica
// sempre ligado). Se o seu exemplar tiver um pino de BL controlável, defina
// aqui (LEDC/PWM); com -1 o dimming de economia de energia é feito por
// overlay de software (ver PowerManager).
constexpr int LCD_PIN_BL   = -1;

constexpr int SCREEN_WIDTH  = 240;
constexpr int SCREEN_HEIGHT = 240;
constexpr uint32_t LCD_SPI_WRITE_HZ = 60'000'000;
constexpr uint32_t LCD_SPI_READ_HZ  = 16'000'000;

// --- IMU QMI8658 (I2C) ---
constexpr int IMU_PIN_SDA  = 47;
constexpr int IMU_PIN_SCL  = 48;
constexpr int IMU_PIN_INT1 = 46;
constexpr int IMU_PIN_INT2 = 45;
constexpr uint8_t IMU_I2C_ADDR = 0x6B;
constexpr uint32_t IMU_I2C_HZ = 400'000;

// ============================================================================
// Timings gerais do fluxo (ms)
// ============================================================================
namespace timing {
constexpr uint32_t SCREEN_DURATION_MS     = 10'000;
constexpr uint32_t TRANSITION_DURATION_MS = 5'000;
constexpr uint32_t BOOT_DURATION_MS       = 5'000;

constexpr uint32_t BITCOIN_POLL_MS  = 15'000;
constexpr uint32_t WEATHER_POLL_MS  = 10 * 60'000;
constexpr uint32_t TIME_SYNC_MS     = 60 * 60'000;
constexpr uint32_t WIFI_RETRY_MS    = 5'000;
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15'000;
constexpr int RAIN_PROBABILITY_THRESHOLD_PCT = 50;

constexpr float TARGET_FPS = 60.0f;
constexpr uint32_t FRAME_INTERVAL_MS = (uint32_t)(1000.0f / TARGET_FPS);
}  // namespace timing

// ============================================================================
// Energia
// ============================================================================
namespace power {
constexpr uint32_t IDLE_TIMEOUT_MS = 3 * 60'000;  // 3 min parado -> dim
constexpr uint8_t DIM_OVERLAY_ALPHA = 90;          // 0..255, preto por cima (queda suave, ~65% do brilho)
constexpr float MOTION_WAKE_THRESHOLD_G = 0.35f;   // delta accel p/ "levantou"
}  // namespace power

// ============================================================================
// Robô / olhos
// ============================================================================
namespace robot {
constexpr uint32_t EYE_COLOR_DEFAULT = 0xFFD800;  // amarelo
constexpr uint32_t EYE_COLORS_ALT[] = {
    0x3AA0FF,  // azul
    0x35E08A,  // verde
    0x36FFF0,  // ciano
    0xFF4A4A,  // vermelho
};
constexpr int EYE_COLORS_ALT_COUNT = 4;
constexpr float MOOD_COLOR_SHIFT_CHANCE = 0.015f;  // por ciclo de blink/idle
}  // namespace robot

}  // namespace board
