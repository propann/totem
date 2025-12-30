#include "TotemUI.h"

#include <math.h>

namespace TotemUI {

void TotemUI::begin() { u8g2.begin(); }

void TotemUI::drawBootScreen(const char *versionTag) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_logisoso24_tr);
    u8g2.drawStr(16, 34, "TOTEM");
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(16, 52, versionTag);
    u8g2.sendBuffer();
}

void TotemUI::update(int joyVal, const String &lastNote) {
    u8g2.clearBuffer();
    drawJoyGauge(joyVal);
    drawNote(lastNote.c_str());
    u8g2.sendBuffer();
}

void TotemUI::drawJoyGauge(int joyValue) {
    const uint8_t barWidth =
        static_cast<uint8_t>(map(joyValue, 0, 1023, 0, 120));
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 14, "JOYSTICK");
    u8g2.drawFrame(4, 18, 120, 10);
    u8g2.drawBox(4, 18, barWidth, 10);

    char valueText[12];
    snprintf(valueText, sizeof(valueText), "%4d", joyValue);
    u8g2.drawStr(96, 12, valueText);
}

void TotemUI::drawNote(const char *label) {
    u8g2.setFont(u8g2_font_logisoso24_tr);
    const int16_t width = u8g2.getStrWidth(label);
    const int16_t x = (128 - width) / 2;
    u8g2.drawStr(x, 60, label);
}

} // namespace TotemUI
