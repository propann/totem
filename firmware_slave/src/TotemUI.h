#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <math.h>

namespace TotemUI {

struct Snapshot {
  bool linkActive = false;
  uint16_t bpm = 120;
  bool drumMode = true;
  int joyValue = 0;
  bool joyActive = false;
  const char *noteLabel = nullptr;
  bool noteActive = false;
  const char *encoderLabel = nullptr;
  uint8_t encoderValue = 0;
  bool encoderActive = false;
  int lastJoyRaw = 0;
  int lastRow = -1;
  int lastCol = -1;
  const char *lastEncDir = "-";
};

inline void formatNoteName(uint8_t note, char *buffer, size_t size) {
  static const char *names[12] = {"C",  "C#", "D",  "D#", "E",  "F",
                                  "F#", "G",  "G#", "A",  "A#", "B"};
  const uint8_t nameIndex = note % 12;
  const int octave = static_cast<int>(note / 12) - 1;
  snprintf(buffer, size, "%s%d", names[nameIndex], octave);
}

class Controller {
 public:
  explicit Controller(U8G2 &display) : u8g2(display) {}

  void begin() { u8g2.begin(); }

  void startBoot(uint32_t now) {
    bootStart = now;
    bootDone = false;
  }

  bool isBootDone() const { return bootDone; }

  void renderBoot(uint32_t now) {
    const uint32_t elapsed = now - bootStart;
    const bool blink = ((elapsed / 200) % 2) == 0;
    const uint16_t progress = elapsed >= 1000 ? 1000 : elapsed;
    const uint8_t barWidth = static_cast<uint8_t>(map(progress, 0, 1000, 0, 120));

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_8x13_tr);
    if (blink) {
      u8g2.drawStr(32, 18, "TOTEM");
    }

    u8g2.drawFrame(4, 28, 120, 8);
    u8g2.drawBox(4, 28, barWidth, 8);

    u8g2.setFont(u8g2_font_6x12_tr);
    if (elapsed < 900) {
      u8g2.drawStr(16, 52, "SYSTEM LINK...");
    } else {
      u8g2.drawStr(40, 52, "READY");
    }
    u8g2.sendBuffer();

    if (elapsed > 1400) {
      bootDone = true;
    }
  }

  void renderDiagnostic(const Snapshot &state) {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 12, "[ DIAGNOSTIC USB ]");

    char line[32];
    snprintf(line, sizeof(line), "JOY : %4d", state.lastJoyRaw);
    u8g2.drawStr(0, 28, line);

    snprintf(line, sizeof(line), "LAST: Touche(%d,%d)", state.lastRow, state.lastCol);
    u8g2.drawStr(0, 42, line);

    snprintf(line, sizeof(line), "ENC : %s", state.lastEncDir);
    u8g2.drawStr(0, 56, line);
    u8g2.sendBuffer();
  }

  void renderDashboard(const Snapshot &state, uint32_t now) {
    u8g2.clearBuffer();
    drawTopBar(state);
    drawFooter(state);

    if (state.noteActive && state.noteLabel) {
      drawNote(state.noteLabel);
    } else if (state.encoderActive && state.encoderLabel) {
      drawEncoderPopup(state.encoderLabel, state.encoderValue);
    } else if (state.joyActive) {
      drawJoyGauge(state.joyValue);
    } else {
      drawOscilloscope(now);
    }

    u8g2.sendBuffer();
  }

 private:
  U8G2 &u8g2;
  uint32_t bootStart = 0;
  bool bootDone = false;

  void drawTopBar(const Snapshot &state) {
    u8g2.setFont(u8g2_font_6x10_tr);
    if (state.linkActive) {
      u8g2.drawStr(0, 8, "LINK");
      u8g2.drawCircle(28, 5, 3);
    } else {
      u8g2.drawStr(0, 8, "X");
    }

    char bpmText[12];
    snprintf(bpmText, sizeof(bpmText), "%3d", state.bpm);
    u8g2.drawStr(108, 8, bpmText);
    u8g2.drawHLine(0, 10, 128);
  }

  void drawFooter(const Snapshot &state) {
    u8g2.drawHLine(0, 52, 128);
    u8g2.setFont(u8g2_font_6x12_tr);
    if (state.drumMode) {
      u8g2.drawStr(0, 62, "[DRUM]");
    } else {
      u8g2.drawStr(0, 62, "[SYNTH]");
    }
  }

  void drawOscilloscope(uint32_t now) {
    const float phase = static_cast<float>(now % 1000) / 80.0f;
    const int16_t midY = 32;
    for (int x = 0; x < 128; x += 4) {
      const float angle = (static_cast<float>(x) / 12.0f) + phase;
      const int y = static_cast<int>(midY + sinf(angle) * 10.0f);
      u8g2.drawPixel(x, y);
      u8g2.drawPixel(x + 1, y);
    }
  }

  void drawJoyGauge(int joyValue) {
    const uint8_t barWidth = static_cast<uint8_t>(map(joyValue, 0, 1023, 0, 100));
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 30, "JOY");
    u8g2.drawFrame(12, 36, 104, 10);
    u8g2.drawBox(12, 36, barWidth, 10);

    char valueText[8];
    snprintf(valueText, sizeof(valueText), "%4d", joyValue);
    u8g2.drawStr(90, 30, valueText);
  }

  void drawNote(const char *label) {
    u8g2.setFont(u8g2_font_logisoso24_tr);
    const int16_t width = u8g2.getStrWidth(label);
    const int16_t x = (128 - width) / 2;
    u8g2.drawStr(x, 46, label);
  }

  void drawEncoderPopup(const char *label, uint8_t value) {
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawFrame(10, 18, 108, 28);
    u8g2.drawStr(18, 30, label);

    const uint8_t barWidth = static_cast<uint8_t>(map(value, 0, 127, 0, 92));
    u8g2.drawFrame(18, 34, 92, 8);
    u8g2.drawBox(18, 34, barWidth, 8);
  }
};

} // namespace TotemUI
