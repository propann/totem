#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

class TotemDisplay {
  public:
    TotemDisplay();

    void begin();
    void drawBootAnim();
    void drawIdle(int joyX, int joyY);
    void drawFeedback(const char *note);

  private:
    void drawWave(int joyX);

    U8G2_SSD1306_128X64_NONAME_F_HW_I2C display;
    float phase = 0.0f;
    unsigned long overlayUntil = 0;
    char overlayNote[8] = "";
};
