/*
 * PROJET TOTEM - FIRMWARE ESCLAVE (CONTROLLER)
 * --------------------------------------------
 * ATTENTION : CE FIRMWARE NECESSITE UNE MODIFICATION PHYSIQUE DU PCB LMN-3 !
 * TABLEAU DE CABLAGE (PIN MAPPING) :
 * -------------------------------------------------------------------------
 * FONCTION          | SIGNAL    | PIN TEENSY ORIGINE | ACTION PHYSIQUE  | DESTINATION / PIN MODIFIEE
 * ------------------|-----------|--------------------|------------------|---------------------------
 * Liaison UART RX   | RX1       | 0                  | PLIER (AIR)      | -> Connecteur Inter-Teensy
 * Liaison UART TX   | TX1       | 1                  | PLIER (AIR)      | -> Connecteur Inter-Teensy
 * Audio S/PDIF      | OUT       | 14                 | PLIER (AIR)      | -> Connecteur Inter-Teensy
 * ------------------|-----------|--------------------|------------------|---------------------------
 * Matrice (Répar.)  | COL_7     | 0 (Conflit)        | FIL VOLANT       | -> Pin 33
 * Matrice (Répar.)  | COL_6     | 1 (Conflit)        | FIL VOLANT       | -> Pin 37
 * Matrice (Répar.)  | COL_9     | 14 (Conflit)       | FIL VOLANT       | -> Pin 38
 * ------------------|-----------|--------------------|------------------|---------------------------
 * Ecran OLED        | SDA       | 18                 | Câblage Direct   | -> Ecran SDA
 * Ecran OLED        | SCL       | 19                 | Câblage Direct   | -> Ecran SCL
 * Joystick (Natif)  | AXE X     | 15 (A1)            | PCB LMN-3 Std    | Pitchbend Natif
 * -------------------------------------------------------------------------
 */

#include <Arduino.h>
#include <Audio.h>
#include <Encoder.h>
#include <SPI.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "config.h"
#include "EncoderMap.h"
#include "KeyMap.h"

// =========================================================
// 1. MODE & INTERFACES
// =========================================================
bool diagnosticMode = false;

// =========================================================
// 2. AUDIO S/PDIF (OSCILLATEUR TEST)
// =========================================================
AudioSynthWaveform testOsc;
AudioOutputSPDIF3 spdif;
AudioConnection patchCordL(testOsc, 0, spdif, 0);
AudioConnection patchCordR(testOsc, 0, spdif, 1);

// =========================================================
// 3. OLED
// =========================================================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// =========================================================
// 4. MATRICE
// =========================================================
static constexpr uint8_t ROW_COUNT = 5;
static constexpr uint8_t COL_COUNT = 10;

const uint8_t ROW_PINS[ROW_COUNT] = {ROW_0, ROW_1, ROW_2, ROW_3, ROW_4};
const uint8_t COL_PINS[COL_COUNT] = {COL_0, COL_1, COL_2, COL_3, COL_4,
                                     COL_5, COL_6, COL_7, COL_8, COL_9};

bool keyState[ROW_COUNT][COL_COUNT] = {};
uint32_t keyLastChange[ROW_COUNT][COL_COUNT] = {};
static constexpr uint32_t KEY_DEBOUNCE_MS = 20;

// =========================================================
// 5. ENCODEURS
// =========================================================
Encoder enc1(ENC1_PIN_A, ENC1_PIN_B);
Encoder enc2(ENC2_PIN_A, ENC2_PIN_B);
Encoder enc3(ENC3_PIN_A, ENC3_PIN_B);
Encoder enc4(ENC4_PIN_A, ENC4_PIN_B);

int enc1Last = 0;
int enc2Last = 0;
int enc3Last = 0;
long enc4Last = 0;

// =========================================================
// 6. ETATS UI
// =========================================================
unsigned long lastJoySend = 0;
unsigned long lastOledUpdate = 0;
int lastJoyRaw = 0;
const char *lastActionLabel = "-";
int lastRowPressed = -1;
int lastColPressed = -1;
const char *lastEncDir = "-";
bool enc4ButtonState = false;
bool enc4ButtonLast = false;
uint32_t enc4ButtonLastChange = 0;
static constexpr uint32_t ENC_BUTTON_DEBOUNCE_MS = 20;

// =========================================================
// 7. UTILITAIRES MIDI
// =========================================================
void sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (diagnosticMode) {
    Serial.print("Row: ");
    Serial.print(row);
    Serial.print(" Col: ");
    Serial.println(col);
    return;
  }
  Serial1.write(0x90 | ((channel - 1) & 0x0F));
  Serial1.write(note);
  Serial1.write(velocity);
}

void sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (diagnosticMode) {
    return;
  }
  Serial1.write(0x80 | ((channel - 1) & 0x0F));
  Serial1.write(note);
  Serial1.write(velocity);
}

void sendCC(uint8_t channel, uint8_t cc, uint8_t value) {
  if (diagnosticMode) {
    return;
  }
  Serial1.write(0xB0 | ((channel - 1) & 0x0F));
  Serial1.write(cc);
  Serial1.write(value);
}

// =========================================================
// 8. INITIALISATION MATRICE
// =========================================================
void setupMatrixPins() {
  for (uint8_t row = 0; row < ROW_COUNT; ++row) {
    pinMode(ROW_PINS[row], OUTPUT);
    digitalWrite(ROW_PINS[row], HIGH);
  }

  for (uint8_t col = 0; col < COL_COUNT; ++col) {
    pinMode(COL_PINS[col], INPUT_PULLUP);
  }
}

void handleKeyChange(uint8_t row, uint8_t col, bool pressed) {
  KeyAction action = getKeyAction(row, col);
  if (action.type == KeyAction::None) {
    return;
  }

  lastActionLabel = action.label ? action.label : "-";
  lastRowPressed = row;
  lastColPressed = col;

  if (diagnosticMode) {
    return;
  }

  if (action.type == KeyAction::Note) {
    if (pressed) {
      sendNoteOn(action.channel, action.number, 100);
    } else {
      sendNoteOff(action.channel, action.number, 0);
    }
  } else if (action.type == KeyAction::Command) {
    sendCC(action.channel, action.number, pressed ? 127 : 0);
  }
}

void scanMatrix() {
  const uint32_t now = millis();

  for (uint8_t row = 0; row < ROW_COUNT; ++row) {
    digitalWrite(ROW_PINS[row], LOW);
    delayMicroseconds(3);

    for (uint8_t col = 0; col < COL_COUNT; ++col) {
      const bool pressed = digitalRead(COL_PINS[col]) == LOW;
      if (pressed != keyState[row][col] && (now - keyLastChange[row][col]) > KEY_DEBOUNCE_MS) {
        keyState[row][col] = pressed;
        keyLastChange[row][col] = now;
        handleKeyChange(row, col, pressed);
      }
    }

    digitalWrite(ROW_PINS[row], HIGH);
  }
}

// =========================================================
// 9. ENCODEURS
// =========================================================
int clampEncoderValue(long value) {
  if (value < ENC_MIN) {
    return ENC_MIN;
  }
  if (value > ENC_MAX) {
    return ENC_MAX;
  }
  return static_cast<int>(value);
}

void handleAbsoluteEncoder(Encoder &enc, int &lastValue, uint8_t cc) {
  long position = enc.read() / ENC_STEPS_PER_DETENT;
  int value = clampEncoderValue(position);
  if (value != lastValue) {
    lastValue = value;
    if (!diagnosticMode) {
      sendCC(MIDI_CH_PERF, cc, static_cast<uint8_t>(value));
    }
  }
}

void handleNavigationEncoder() {
  long position = enc4.read() / ENC_STEPS_PER_DETENT;
  long delta = position - enc4Last;
  if (delta == 0) {
    return;
  }

  enc4Last = position;
  if (delta > 0) {
    lastEncDir = "> Droite";
    if (!diagnosticMode) {
      sendCC(MIDI_CH_PERF, CMD_NEXT, 127);
      sendCC(MIDI_CH_PERF, CMD_NEXT, 0);
    }
  } else {
    lastEncDir = "< Gauche";
    if (!diagnosticMode) {
      sendCC(MIDI_CH_PERF, CMD_PREV, 127);
      sendCC(MIDI_CH_PERF, CMD_PREV, 0);
    }
  }
}

void handleEncoderButton() {
  const uint32_t now = millis();
  const bool pressed = digitalRead(ENC4_BUTTON_PIN) == LOW;
  if (pressed != enc4ButtonLast && (now - enc4ButtonLastChange) > ENC_BUTTON_DEBOUNCE_MS) {
    enc4ButtonLastChange = now;
    enc4ButtonLast = pressed;
    enc4ButtonState = pressed;
    if (!diagnosticMode) {
      sendCC(MIDI_CH_PERF, CMD_ENTER, pressed ? 127 : 0);
    }
  }
}

// =========================================================
// 10. OLED UI
// =========================================================
void drawBootScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 14, "PROJET TOTEM");
  u8g2.drawStr(0, 30, "INITIALISATION");
  u8g2.drawStr(0, 46, "FIRMWARE ESCLAVE");
  u8g2.sendBuffer();
}

void drawDiagnosticScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 12, "[ DIAGNOSTIC USB ]");

  char line[32];
  snprintf(line, sizeof(line), "JOY : %4d", lastJoyRaw);
  u8g2.drawStr(0, 28, line);

  snprintf(line, sizeof(line), "LAST: Touche(%d,%d)", lastRowPressed, lastColPressed);
  u8g2.drawStr(0, 42, line);

  snprintf(line, sizeof(line), "ENC : %s", lastEncDir);
  u8g2.drawStr(0, 56, line);
  u8g2.sendBuffer();
}

void drawNormalScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 12, "TOTEM [Esclave]");
  u8g2.drawStr(0, 26, "S/PDIF: ON");
  u8g2.drawHLine(0, 32, 128);

  char line[32];
  snprintf(line, sizeof(line), "Note: %s", lastActionLabel);
  u8g2.drawStr(0, 50, line);
  u8g2.sendBuffer();
}

// =========================================================
// 11. SETUP & LOOP
// =========================================================
void setup() {
  pinMode(ENC4_BUTTON_PIN, INPUT_PULLUP);
  diagnosticMode = digitalRead(ENC4_BUTTON_PIN) == LOW;

  if (diagnosticMode) {
    Serial.begin(DEBUG_BAUDRATE);
    while (!Serial && millis() < 2000) {
      delay(10);
    }
    Serial.println("[TOTEM] MODE DIAGNOSTIC USB");
  } else {
    Serial1.begin(2000000);
  }

  setupMatrixPins();

  AudioMemory(10);
  testOsc.begin(WAVEFORM_SINE);
  testOsc.frequency(440.0f);
  testOsc.amplitude(0.2f);

  u8g2.begin();
  drawBootScreen();
  delay(1200);
}

void loop() {
  scanMatrix();

  handleAbsoluteEncoder(enc1, enc1Last, CC_FILTER);
  handleAbsoluteEncoder(enc2, enc2Last, CC_RESO);
  handleAbsoluteEncoder(enc3, enc3Last, CC_FX);
  handleNavigationEncoder();
  handleEncoderButton();

  const uint32_t now = millis();

  if (!diagnosticMode && now - lastJoySend >= 15) {
    lastJoySend = now;
    lastJoyRaw = analogRead(PIN_JOY_MAIN);
    uint8_t xMapped = static_cast<uint8_t>(map(lastJoyRaw, 0, 1023, 0, 254));

    Serial1.write(0xFF);
    Serial1.write(xMapped);
    Serial1.write(static_cast<uint8_t>(127));
    Serial1.write(static_cast<uint8_t>(0));
  }

  if (diagnosticMode && now - lastJoySend >= 15) {
    lastJoySend = now;
    lastJoyRaw = analogRead(PIN_JOY_MAIN);
    Serial.print("Joy: ");
    Serial.println(lastJoyRaw);
  }

  if (now - lastOledUpdate >= 50) {
    lastOledUpdate = now;
    if (diagnosticMode) {
      drawDiagnosticScreen();
    } else {
      drawNormalScreen();
    }
  }
}
