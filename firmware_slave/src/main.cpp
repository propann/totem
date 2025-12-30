#include <Arduino.h>
#include <U8g2lib.h>

// 1. SETUP ECRAN
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE,
                                        /* clock=*/ 19, /* data=*/ 18);

// 2. SETUP MATRICE (Hack Pinout)
const uint8_t colPins[] = {9, 8, 7, 4, 3, 2, 37, 33, 25, 38};
const uint8_t rowPins[] = {24, 23, 34, 35, 28};

static constexpr uint8_t ROWS = sizeof(rowPins) / sizeof(rowPins[0]);
static constexpr uint8_t COLS = sizeof(colPins) / sizeof(colPins[0]);

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
  Serial.begin(9600);
  u8g2.begin();
  setupMatrixPins();
  pinMode(A1, INPUT); // Joystick
}

void loop() {
  scanMatrix();

  u8g2.clearBuffer();

  // Titre
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "TOTEM SLAVE TEST");

  // Affichage Joystick
  int joyVal = analogRead(A1);
  const uint8_t barUnits = map(joyVal, 0, 1023, 0, 10);
  char joyLine[32];
  char bar[11];
  for (uint8_t i = 0; i < 10; ++i) {
    bar[i] = (i < barUnits) ? '=' : '.';
  }
  bar[10] = '\0';
  snprintf(joyLine, sizeof(joyLine), "J: [%s] (%d)", bar, joyVal);
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 28, joyLine);

  // Affichage Matrice (Dernière touche détectée)
  char keyLine[24];
  if (lastRowPressed >= 0 && lastColPressed >= 0) {
    snprintf(keyLine, sizeof(keyLine), "KEY: R[%d] C[%d]",
             lastRowPressed, lastColPressed);
  } else {
    snprintf(keyLine, sizeof(keyLine), "KEY: R[-] C[-]");
  }
  u8g2.drawStr(0, 44, keyLine);

  u8g2.drawStr(0, 62, "Hack Pins: 33/37/38");

  u8g2.sendBuffer();
}
