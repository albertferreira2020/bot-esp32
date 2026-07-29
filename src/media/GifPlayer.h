#pragma once
#include <AnimatedGIF.h>
#include "display/DisplayManager.h"

// Player de GIF não-bloqueante a partir do LittleFS. Decodifica um frame por
// chamada de update() (nunca trava o loop principal); se o arquivo não
// existir, begin() retorna false e quem chamou cai no efeito procedural
// equivalente.
class GifPlayer {
   public:
    bool begin(DisplayManager& display, const char* path, int cx, int cy);
    void update(uint32_t nowMs);
    void end();

    bool isOpen() const { return isOpen_; }
    int width() const { return gifWidth_; }
    int height() const { return gifHeight_; }
    Rect lastDirtyRect() const { return dirty_; }

   private:
    static void drawCallback(GIFDRAW* pDraw);
    void onDrawLine(GIFDRAW* pDraw);

    AnimatedGIF gif_;
    DisplayManager* display_ = nullptr;
    int originX_ = 0, originY_ = 0;
    int gifWidth_ = 0, gifHeight_ = 0;
    bool isOpen_ = false;
    uint32_t nextFrameAt_ = 0;
    Rect dirty_;
};
