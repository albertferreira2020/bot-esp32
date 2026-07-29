#include "media/GifPlayer.h"

#include <Arduino.h>
#include <LittleFS.h>

namespace {
File s_gifFile;
GifPlayer* s_activeInstance = nullptr;

void* gifOpen(const char* filename, int32_t* pFileSize) {
    s_gifFile = LittleFS.open(filename, "r");
    if (!s_gifFile) return nullptr;
    *pFileSize = s_gifFile.size();
    return &s_gifFile;
}

void gifClose(void* pHandle) {
    File* f = static_cast<File*>(pHandle);
    if (f) f->close();
}

int32_t gifRead(GIFFILE* pFile, uint8_t* pBuf, int32_t iLen) {
    File* f = static_cast<File*>(pFile->fHandle);
    int32_t avail = pFile->iSize - pFile->iPos;
    if (avail <= 0) return 0;
    if (iLen > avail) iLen = avail;
    int32_t got = f->read(pBuf, iLen);
    if (got > 0) pFile->iPos += got;
    return got;
}

int32_t gifSeek(GIFFILE* pFile, int32_t iPosition) {
    File* f = static_cast<File*>(pFile->fHandle);
    f->seek(iPosition);
    pFile->iPos = iPosition;
    return iPosition;
}
}  // namespace

bool GifPlayer::begin(DisplayManager& display, const char* path, int cx, int cy) {
    end();
    display_ = &display;
    gif_.begin(GIF_PALETTE_RGB565_LE);

    if (!gif_.open(path, gifOpen, gifClose, gifRead, gifSeek, &GifPlayer::drawCallback)) {
        isOpen_ = false;
        return false;
    }

    gifWidth_ = gif_.getCanvasWidth();
    gifHeight_ = gif_.getCanvasHeight();
    originX_ = cx - gifWidth_ / 2;
    originY_ = cy - gifHeight_ / 2;
    isOpen_ = true;
    nextFrameAt_ = 0;
    return true;
}

void GifPlayer::end() {
    if (isOpen_) gif_.close();
    isOpen_ = false;
}

void GifPlayer::drawCallback(GIFDRAW* pDraw) {
    if (s_activeInstance) s_activeInstance->onDrawLine(pDraw);
}

void GifPlayer::onDrawLine(GIFDRAW* pDraw) {
    int destY = originY_ + pDraw->iY + pDraw->y;
    uint16_t* row = display_->rawRow(destY);
    if (!row) return;

    int destXStart = originX_ + pDraw->iX;
    const uint8_t* src = pDraw->pPixels;
    const uint16_t* pal = pDraw->pPalette;
    int screenW = DisplayManager::width();

    Rect line{destXStart, destY, pDraw->iWidth, 1};
    dirty_ = (dirty_.w == 0) ? line : dirty_.united(line);

    for (int i = 0; i < pDraw->iWidth; ++i) {
        int dx = destXStart + i;
        if (dx < 0 || dx >= screenW) continue;
        uint8_t idx = src[i];
        if (pDraw->ucHasTransparency && idx == pDraw->ucTransparent) continue;
        // Paleta vem em RGB565 padrão (GIF_PALETTE_RGB565_LE); o back-buffer
        // espera swap565 — daí o toBufferOrder.
        row[dx] = DisplayManager::toBufferOrder(display_->tint(pal[idx]));
    }
}

void GifPlayer::update(uint32_t nowMs) {
    if (!isOpen_) return;
    if (nextFrameAt_ != 0 && nowMs < nextFrameAt_) return;

    dirty_ = Rect{};
    s_activeInstance = this;
    int delayMs = 0;
    int result = gif_.playFrame(false, &delayMs, nullptr);
    s_activeInstance = nullptr;

    if (result != 1) gif_.reset();  // fim da animação: loop
    nextFrameAt_ = nowMs + (uint32_t)(delayMs > 0 ? delayMs : 40);
}
