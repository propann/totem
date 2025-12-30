#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>

namespace TotemUI {

class TotemUI {
  public:
    explicit TotemUI(U8G2 &display) : u8g2(display) {}

    void begin();
    void drawBootScreen(const char *versionTag);
    void update(int joyVal, const String &lastNote);

  private:
    U8G2 &u8g2;

    void drawJoyGauge(int joyValue);
    void drawNote(const char *label);
};

} // namespace TotemUI
