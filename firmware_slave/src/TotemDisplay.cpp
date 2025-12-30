#include "TotemDisplay.h"

#include <math.h>
#include "config.h"

TotemDisplay::TotemDisplay()
    : display(U8G2_R0, /* reset=*/U8X8_PIN_NONE,
              /* clock=*/19, /* data=*/18) {}

void TotemDisplay::begin() {
    display.begin();
    display.setPowerSave(0);
    display.setContrast(255);
}

void TotemDisplay::drawBootAnim() {
    const unsigned long start = millis();
    while (millis() - start < 2000) {
        display.clearBuffer();
        display.setFont(u8g2_font_logisoso24_tr);
        display.drawStr(10, 36, "TOTEM");
        display.setFont(u8g2_font_6x12_tr);
        display.drawStr(10, 52, "SLAVE");
        display.setFont(u8g2_font_6x12_tr);
        display.drawStr(10, 56, SLAVE_VERSION);
        display.drawStr(10, 68, SLAVE_HACK_TAG);
        // Effet de remplissage simple
        const uint8_t bar = static_cast<uint8_t>(((millis() - start) % 500) /
                                                 500.0f * 128);
        display.drawBox(0, 50, bar, 4);
        display.sendBuffer();
        delay(40);
    }
    strncpy(overlayNote, "READY", sizeof(overlayNote) - 1);
    overlayNote[sizeof(overlayNote) - 1] = '\0';
    overlayUntil = millis() + 1000;
    overlayMode = Overlay::Note;
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

void TotemDisplay::drawPulseBar() {
    const uint8_t width =
        static_cast<uint8_t>((sinf(millis() / 300.0f) + 1.0f) * 0.5f * 120);
    display.drawFrame(4, 56, 120, 6);
    display.drawBox(4, 56, width, 6);
}

void TotemDisplay::drawOverlay() {
    if (!overlayUntil || millis() > overlayUntil) {
        overlayMode = Overlay::None;
        return;
    }
    switch (overlayMode) {
    case Overlay::Note:
        display.setFont(u8g2_font_logisoso18_tr);
        display.drawStr(6, 42, "NOTE:");
        display.drawStr(70, 42, overlayNote);
        break;
    case Overlay::Joy: {
        display.setFont(u8g2_font_6x12_tr);
        char buf[18];
        snprintf(buf, sizeof(buf), "JOY:%4d", overlayJoy);
        display.drawStr(6, 28, buf);
        const uint8_t w = static_cast<uint8_t>(map(overlayJoy, 0, 1023, 0, 120));
        display.drawFrame(4, 36, 120, 8);
        display.drawBox(4, 36, w, 8);
        break;
    }
    case Overlay::Enc: {
        display.setFont(u8g2_font_6x12_tr);
        char buf[24];
        snprintf(buf, sizeof(buf), "ENC%d:%ld", overlayEncId, overlayEncVal);
        display.drawStr(6, 40, buf);
        break;
    }
    default:
        break;
    }
}

void TotemDisplay::update(int joyX) {
    display.clearBuffer();
    drawWave(joyX);
    drawPulseBar();
    drawOverlay();
    display.sendBuffer();
}

void TotemDisplay::setNoteFeedback(const char *note) {
    strncpy(overlayNote, note, sizeof(overlayNote) - 1);
    overlayNote[sizeof(overlayNote) - 1] = '\0';
    overlayUntil = millis() + 600; // affiche 600 ms
    overlayMode = Overlay::Note;
}

void TotemDisplay::setJoystickValue(int joy) {
    overlayJoy = joy;
    overlayUntil = millis() + 400;
    overlayMode = Overlay::Joy;
}

void TotemDisplay::setEncoderValue(uint8_t id, long value) {
    overlayEncId = id;
    overlayEncVal = value;
    overlayUntil = millis() + 600;
    overlayMode = Overlay::Enc;
}
