#include "TotemDisplay.h"

#include <math.h>

TotemDisplay::TotemDisplay()
    : display(U8G2_R0, /* reset=*/U8X8_PIN_NONE,
              /* clock=*/19, /* data=*/18) {}

void TotemDisplay::begin() { display.begin(); }

void TotemDisplay::drawBootAnim() {
    const unsigned long start = millis();
    while (millis() - start < 2000) {
        display.clearBuffer();
        display.setFont(u8g2_font_logisoso24_tr);
        display.drawStr(18, 40, "TOTEM");
        // Effet de remplissage simple
        const uint8_t bar = static_cast<uint8_t>(((millis() - start) % 500) /
                                                 500.0f * 128);
        display.drawBox(0, 50, bar, 4);
        display.sendBuffer();
        delay(40);
    }
}

void TotemDisplay::drawWave(int joyX) {
    display.setDrawColor(1);
    const float amplitude =
        4.0f + (static_cast<float>(joyX) / 1023.0f) * 14.0f; // 4..18 px
    const float freq =
        0.8f + (static_cast<float>(joyX) / 1023.0f) * 2.2f; // 0.8..3.0
    for (int x = 0; x < 128; ++x) {
        const float y = 32.0f + amplitude * sinf(freq * (x / 20.0f) + phase);
        display.drawPixel(x, static_cast<int>(y));
    }
    phase += 0.08f;
}

void TotemDisplay::drawIdle(int joyX, int joyY) {
    (void)joyY; // Y pas câblé mais gardé pour évolution
    display.clearBuffer();
    drawWave(joyX);

    // Overlay note si actif
    if (overlayUntil && millis() < overlayUntil) {
        display.setFont(u8g2_font_logisoso18_tr);
        const int16_t width = display.getStrWidth(overlayNote);
        const int16_t x = (128 - width) / 2;
        display.drawStr(x, 44, overlayNote);
    }

    display.sendBuffer();
}

void TotemDisplay::drawFeedback(const char *note) {
    strncpy(overlayNote, note, sizeof(overlayNote) - 1);
    overlayNote[sizeof(overlayNote) - 1] = '\0';
    overlayUntil = millis() + 600; // affiche 600 ms
}
