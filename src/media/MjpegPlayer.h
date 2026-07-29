#pragma once
#include <Arduino.h>
#include "display/DisplayManager.h"

// Player de sequências MJPEG (JPEGs numerados 240x240 + manifest.json) vindas
// do LittleFS, geradas por tools/pinterest_to_mjpeg.py. Sempre em tela cheia
// (transições/chuva). Se a pasta/manifest não existir, begin() retorna false
// e quem chamou usa o efeito procedural equivalente.
class MjpegPlayer {
   public:
    bool begin(DisplayManager& display, const char* folder);
    void update(uint32_t nowMs);
    void end();

    bool isOpen() const { return isOpen_; }
    Rect lastDirtyRect() const { return dirty_; }

   private:
    static bool jpegOutputCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* data);
    void onBlock(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* data);

    DisplayManager* display_ = nullptr;
    String folder_;
    int frameCount_ = 0;
    int fps_ = 30;
    bool loop_ = true;
    int currentFrame_ = 0;
    uint32_t frameIntervalMs_ = 33;
    uint32_t nextFrameAt_ = 0;
    bool isOpen_ = false;
    bool warnedMissingFrame_ = false;

    uint8_t* scratch_ = nullptr;
    size_t scratchCap_ = 0;

    Rect dirty_;
};
