#include "media/MjpegPlayer.h"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <TJpg_Decoder.h>

namespace {
MjpegPlayer* s_active = nullptr;
}

bool MjpegPlayer::begin(DisplayManager& display, const char* folder) {
    end();
    display_ = &display;
    folder_ = folder;

    String manifestPath = folder_ + "/manifest.json";
    File mf = LittleFS.open(manifestPath.c_str(), "r");
    if (!mf) {
        Serial.printf("[MjpegPlayer] %s nao encontrado (rodou 'pio run -t uploadfs'?)\n",
                      manifestPath.c_str());
        return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, mf);
    mf.close();
    if (err) {
        Serial.printf("[MjpegPlayer] %s: manifest.json invalido (%s)\n", folder_.c_str(), err.c_str());
        return false;
    }

    fps_ = doc["fps"] | 30;
    frameCount_ = doc["frames"] | 0;
    loop_ = doc["loop"] | true;
    if (frameCount_ <= 0) {
        Serial.printf("[MjpegPlayer] %s: manifest.json sem 'frames' valido\n", folder_.c_str());
        return false;
    }
    Serial.printf("[MjpegPlayer] %s: %d frames @ %d fps\n", folder_.c_str(), frameCount_, fps_);

    frameIntervalMs_ = 1000 / (uint32_t)(fps_ > 0 ? fps_ : 30);
    currentFrame_ = 0;
    nextFrameAt_ = 0;
    isOpen_ = true;
    warnedMissingFrame_ = false;

    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(false);
    TJpgDec.setCallback(&MjpegPlayer::jpegOutputCallback);
    return true;
}

void MjpegPlayer::end() {
    isOpen_ = false;
    if (scratch_) {
        free(scratch_);
        scratch_ = nullptr;
        scratchCap_ = 0;
    }
}

bool MjpegPlayer::jpegOutputCallback(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* data) {
    if (s_active) s_active->onBlock(x, y, w, h, data);
    return true;
}

void MjpegPlayer::onBlock(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* data) {
    int screenW = DisplayManager::width();
    for (int row = 0; row < h; ++row) {
        uint16_t* dst = display_->rawRow(y + row);
        if (!dst) continue;
        const uint16_t* srcRow = data + (size_t)row * w;
        for (int col = 0; col < w; ++col) {
            int dx = x + col;
            if (dx < 0 || dx >= screenW) continue;
            dst[dx] = display_->tint(srcRow[col]);
        }
    }
}

void MjpegPlayer::update(uint32_t nowMs) {
    if (!isOpen_ || frameCount_ <= 0) return;
    if (nextFrameAt_ != 0 && nowMs < nextFrameAt_) return;

    char path[96];
    snprintf(path, sizeof(path), "%s/frame_%05d.jpg", folder_.c_str(), currentFrame_ + 1);

    File f = LittleFS.open(path, "r");
    if (f) {
        size_t sz = f.size();
        if (sz > scratchCap_) {
            if (scratch_) free(scratch_);
            scratch_ = (uint8_t*)ps_malloc(sz);
            scratchCap_ = scratch_ ? sz : 0;
        }
        if (scratch_ && scratchCap_ >= sz) {
            f.read(scratch_, sz);
            f.close();
            dirty_ = Rect{0, 0, DisplayManager::width(), DisplayManager::height()};
            s_active = this;
            TJpgDec.drawJpg(0, 0, scratch_, sz);
            s_active = nullptr;
        } else {
            f.close();
            Serial.printf("[MjpegPlayer] %s: sem PSRAM pra %u bytes\n", path, (unsigned)sz);
        }
    } else if (!warnedMissingFrame_) {
        warnedMissingFrame_ = true;
        Serial.printf("[MjpegPlayer] %s: frame nao encontrado\n", path);
    }

    currentFrame_++;
    if (currentFrame_ >= frameCount_) {
        if (loop_) {
            currentFrame_ = 0;
        } else {
            isOpen_ = false;
        }
    }
    nextFrameAt_ = nowMs + frameIntervalMs_;
}
