#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

class TotemDisplay {
  public:
    TotemDisplay();

    void begin();
    void drawBootAnim();
    void setNoteFeedback(const char *note);
    void setJoystickValue(int joy);
    void setEncoderValue(uint8_t id, long value);
    void update(int joyVal);

  private:
    enum class Overlay { None, Note, Joy, Enc };

    void drawWave(int joyX);
    void drawPulseBar();
    void drawOverlay();

    U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;
    float phase = 0.0f;
    unsigned long overlayUntil = 0;
    Overlay overlayMode = Overlay::None;
    char overlayNote[8] = "";
    int overlayJoy = 0;
    uint8_t overlayEncId = 0;
    long overlayEncVal = 0;
};
