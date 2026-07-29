#include "display/DisplayManager.h"

LGFX::LGFX() {
    {
        auto cfg = _bus.config();
        cfg.spi_host = SPI2_HOST;
        cfg.spi_mode = 0;
        cfg.freq_write = board::LCD_SPI_WRITE_HZ;
        cfg.freq_read = board::LCD_SPI_READ_HZ;
        cfg.spi_3wire = false;
        cfg.use_lock = true;
        cfg.dma_channel = SPI_DMA_CH_AUTO;
        cfg.pin_sclk = board::LCD_PIN_SCLK;
        cfg.pin_mosi = board::LCD_PIN_MOSI;
        cfg.pin_miso = -1;
        cfg.pin_dc = board::LCD_PIN_DC;
        _bus.config(cfg);
        _panel.setBus(&_bus);
    }
    {
        auto cfg = _panel.config();
        cfg.pin_cs = board::LCD_PIN_CS;
        cfg.pin_rst = board::LCD_PIN_RST;
        cfg.pin_busy = -1;
        cfg.memory_width = board::SCREEN_WIDTH;
        cfg.memory_height = board::SCREEN_HEIGHT;
        cfg.panel_width = board::SCREEN_WIDTH;
        cfg.panel_height = board::SCREEN_HEIGHT;
        // Se a imagem aparecer deslocada/cortada no seu exemplar, ajuste
        // offset_x/offset_y/offset_rotation aqui.
        cfg.offset_x = 0;
        cfg.offset_y = 0;
        cfg.offset_rotation = 2;  // painel monta com a imagem de cabeça pra baixo (0=0°,1=90°,2=180°,3=270°)
        cfg.readable = false;
        cfg.invert = true;  // maioria dos módulos ST7789 240x240 precisa disso
        cfg.rgb_order = false;
        cfg.dlen_16bit = false;
        cfg.bus_shared = false;
        _panel.config(cfg);
    }
    setPanel(&_panel);
}

void DisplayManager::begin() {
    lgfx_.init();
    lgfx_.setColorDepth(16);
    lgfx_.setBrightness(255);
    lgfx_.fillScreen(TFT_BLACK);

    if constexpr (board::LCD_PIN_BL >= 0) {
        pinMode(board::LCD_PIN_BL, OUTPUT);
        digitalWrite(board::LCD_PIN_BL, HIGH);
    }

    canvas_.setPsram(true);
    canvas_.setColorDepth(lgfx::color_depth_t::rgb565_2Byte);
    canvas_.createSprite(width(), height());
    canvas_.fillScreen(TFT_BLACK);
    pushFull();
}

void DisplayManager::clear() { canvas_.fillScreen(TFT_BLACK); }

void DisplayManager::pushFull() {
    lgfx_.startWrite();
    canvas_.pushSprite(&lgfx_, 0, 0);
    lgfx_.endWrite();
}

void DisplayManager::pushRegion(int x, int y, int w, int h) {
    if (w <= 0 || h <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > width()) w = width() - x;
    if (y + h > height()) h = height() - y;
    if (w <= 0 || h <= 0) return;

    auto* buf = static_cast<uint16_t*>(canvas_.getBuffer());
    if (!buf) { pushFull(); return; }

    lgfx_.startWrite();
    for (int row = 0; row < h; ++row) {
        const uint16_t* rowPtr = buf + (size_t)(y + row) * width() + x;
        lgfx_.setAddrWindow(x, y + row, w, 1);
        lgfx_.writePixels(rowPtr, w, false);
    }
    lgfx_.endWrite();
}

void DisplayManager::setBrightness(float scale) {
    if (scale < 0.0f) scale = 0.0f;
    if (scale > 1.0f) scale = 1.0f;
    brightness_ = scale;
}

uint16_t DisplayManager::tint(uint32_t rgb888) const {
    uint8_t r = (rgb888 >> 16) & 0xFF;
    uint8_t g = (rgb888 >> 8) & 0xFF;
    uint8_t b = rgb888 & 0xFF;
    r = (uint8_t)(r * brightness_);
    g = (uint8_t)(g * brightness_);
    b = (uint8_t)(b * brightness_);
    return lgfx::color565(r, g, b);
}

uint16_t* DisplayManager::rawRow(int y) {
    if (y < 0 || y >= height()) return nullptr;
    auto* buf = static_cast<uint16_t*>(canvas_.getBuffer());
    if (!buf) return nullptr;
    return buf + (size_t)y * width();
}

uint16_t DisplayManager::tint(uint16_t rgb565) const {
    if (brightness_ >= 0.999f) return rgb565;
    uint8_t r = (rgb565 >> 11) & 0x1F;
    uint8_t g = (rgb565 >> 5) & 0x3F;
    uint8_t b = rgb565 & 0x1F;
    r = (uint8_t)(r * brightness_);
    g = (uint8_t)(g * brightness_);
    b = (uint8_t)(b * brightness_);
    return (uint16_t)((r << 11) | (g << 5) | b);
}
