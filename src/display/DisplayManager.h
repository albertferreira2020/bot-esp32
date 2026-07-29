#pragma once
#include <LovyanGFX.hpp>
#include "config/BoardConfig.h"

struct Rect {
    int x = 0, y = 0, w = 0, h = 0;

    Rect united(const Rect& other) const {
        if (w <= 0 || h <= 0) return other;
        if (other.w <= 0 || other.h <= 0) return *this;
        int nx = x < other.x ? x : other.x;
        int ny = y < other.y ? y : other.y;
        int nx2 = (x + w) > (other.x + other.w) ? (x + w) : (other.x + other.w);
        int ny2 = (y + h) > (other.y + other.h) ? (y + h) : (other.y + other.h);
        return Rect{nx, ny, nx2 - nx, ny2 - ny};
    }
};

// Painel LGFX custom para o Waveshare ESP32-S3-LCD-1.3 (ST7789 240x240 SPI).
class LGFX : public lgfx::LGFX_Device {
   public:
    LGFX();

   private:
    lgfx::Panel_ST7789 _panel;
    lgfx::Bus_SPI _bus;
};

// Camada única de acesso ao display: dono do LGFX + do back-buffer em PSRAM.
// Toda tela desenha em canvas() e usa tint()/tintRGB() para já sair com o
// brilho de economia de energia aplicado, depois manda pushFull()/pushRegion()
// para efetivamente atualizar o painel físico.
class DisplayManager {
   public:
    void begin();

    lgfx::LGFX_Sprite& canvas() { return canvas_; }
    LGFX& raw() { return lgfx_; }

    static constexpr int width() { return board::SCREEN_WIDTH; }
    static constexpr int height() { return board::SCREEN_HEIGHT; }

    void clear();
    void pushFull();
    // Atualização parcial: copia só o retângulo (clipado à tela) do back-buffer
    // para o painel físico, linha a linha (mais barato que reenviar tudo).
    void pushRegion(int x, int y, int w, int h);

    // Economia de energia: 1.0 = brilho total, 0.0 = preto. Sem GPIO de
    // backlight confirmado nesta placa, o dimming é aplicado nas próprias
    // cores desenhadas (ver tint()).
    void setBrightness(float scale);
    float brightness() const { return brightness_; }

    uint16_t tint(uint32_t rgb888) const;
    uint16_t tint(uint16_t rgb565) const;

    // Acesso direto a uma linha do back-buffer, para players (GIF/MJPEG) que
    // precisam escrever pixel a pixel. nullptr se y fora da tela.
    uint16_t* rawRow(int y);

    // O back-buffer é rgb565_2Byte, que no LovyanGFX é o layout "swap565"
    // (GGGBBBBB RRRRRGGG) — os bytes vêm trocados em relação ao RGB565
    // padrão. Quem desenha pela API do LovyanGFX não se preocupa (ela
    // converte), mas quem escreve direto via rawRow() PRECISA passar a cor
    // por aqui, senão as cores saem distorcidas.
    static uint16_t toBufferOrder(uint16_t rgb565) { return __builtin_bswap16(rgb565); }

   private:
    LGFX lgfx_;
    lgfx::LGFX_Sprite canvas_{&lgfx_};
    float brightness_ = 1.0f;
};
