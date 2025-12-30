#pragma once

// Compatibility shim to keep existing code working while driving a 480x320
// panel with the ILI9488 driver.
#include <ILI9488_t3.h>

class ILI9341_t3n : public ILI9488_t3 {
  public:
    using ILI9488_t3::ILI9488_t3;

    // The original code toggles this flag around certain drawing operations.
    bool console = false;

    // The original driver exposed these helpers; here we provide light stubs.
    void flushSysEx() {}
    void brightness(uint8_t) {}
    void fillRectRainbow(uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                         uint16_t) {
        // Fallback: simple fill since the gradient helper doesn't exist here.
        fillRect(x, y, w, h, ILI9488_WHITE);
    }
};
