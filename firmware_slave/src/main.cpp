#include <Arduino.h>
#include <Control_Surface.h>
#include <U8g2lib.h>

// 1. SETUP ECRAN
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE,
                                        /* clock=*/ 19, /* data=*/ 18);

// 2. SETUP MATRICE (Hack Pinout)
const pin_t colPins[] = {9, 8, 7, 4, 3, 2, 37, 33, 25, 38};
const pin_t rowPins[] = {24, 23, 34, 35, 28};

static constexpr uint8_t ROWS = sizeof(rowPins) / sizeof(rowPins[0]);
static constexpr uint8_t COLS = sizeof(colPins) / sizeof(colPins[0]);

const uint8_t matrixNotes[ROWS][COLS] = {
  {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
  {11, 12, 13, 14, 15, 16, 17, 18, 19, 20},
  {21, 22, 23, 24, 25, 26, 27, 28, 29, 30},
  {31, 32, 33, 34, 35, 36, 37, 38, 39, 40},
  {41, 42, 43, 44, 45, 46, 47, 48, 49, 50},
};

bool keyState[ROWS][COLS] = {};
uint32_t keyLastChange[ROWS][COLS] = {};
static constexpr uint32_t KEY_DEBOUNCE_MS = 20;

int lastRowPressed = -1;
int lastColPressed = -1;

// 3. LOGIQUE
void setupMatrixPins() {
  for (uint8_t row = 0; row < ROWS; ++row) {
    pinMode(rowPins[row], OUTPUT);
    digitalWrite(rowPins[row], HIGH);
  }

  for (uint8_t col = 0; col < COLS; ++col) {
    pinMode(colPins[col], INPUT_PULLUP);
  }
}

void scanMatrix() {
  const uint32_t now = millis();

  for (uint8_t row = 0; row < ROWS; ++row) {
    digitalWrite(rowPins[row], LOW);
    delayMicroseconds(3);

    for (uint8_t col = 0; col < COLS; ++col) {
      const bool pressed = digitalRead(colPins[col]) == LOW;
      if (pressed != keyState[row][col] &&
          (now - keyLastChange[row][col]) > KEY_DEBOUNCE_MS) {
        keyState[row][col] = pressed;
        keyLastChange[row][col] = now;
        if (pressed) {
          lastRowPressed = static_cast<int>(row);
          lastColPressed = static_cast<int>(col);
        }
      }
    }

    digitalWrite(rowPins[row], HIGH);
  }
}

void setup() {
  u8g2.begin();
  setupMatrixPins();
  pinMode(A1, INPUT); // Joystick
}

void loop() {
  scanMatrix();

  u8g2.clearBuffer();

  // Titre
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "TOTEM DIAG");

  // Affichage Joystick
  int joyVal = analogRead(A1);
  const int barWidth = map(joyVal, 0, 1023, 0, 98);
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 30, "JOY:");
  u8g2.drawFrame(30, 22, 98, 10);
  u8g2.drawBox(30, 22, barWidth, 10);

  // Affichage Matrice (Dernière touche détectée)
  if (lastRowPressed >= 0 && lastColPressed >= 0) {
    char buf[32];
    snprintf(buf, sizeof(buf), "TOUCH: Row %d - Col %d",
             lastRowPressed + 1, lastColPressed + 1);
    u8g2.drawStr(0, 50, buf);
  } else {
    u8g2.drawStr(0, 50, "TOUCH: -");
  }

  u8g2.sendBuffer();
}
