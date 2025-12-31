#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

// ------------------- Matrice Easy-Wire -------------------
constexpr uint8_t ROWS[5] = {24, 23, 34, 35, 28};
constexpr uint8_t COLS[14] = {9,  8,  7,  4,  3,  2,  16,
                              33, 25, 17, 13, 41, 40, 36};
const uint8_t NOTE_MAP[5][14] = {
    {36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49},
    {50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63},
    {64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77},
    {78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91},
    {92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105},
};
bool prevState[5][14] = {};

// ------------------- Potentiomètres (ajuste les pins si besoin) -------------
const uint8_t POT_PINS[] = {A0, A1, A2, A3};
constexpr size_t POT_COUNT = sizeof(POT_PINS) / sizeof(POT_PINS[0]);
int potPrev[POT_COUNT];

// Définition des deux écrans (Mode Hardware I2C, Pas de Reset)
// Note : On utilise le même constructeur pour les deux, rotation R2 (180°)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C leftEye(U8G2_R2, U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C rightEye(U8G2_R2, U8X8_PIN_NONE);

// ------------------- Helpers -------------------
void noteNameFromNumber(uint8_t note, char *buf, size_t len) {
  static const char *names[12] = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
  const uint8_t idx = note % 12;
  const int octave = static_cast<int>(note / 12) - 1;
  snprintf(buf, len, "%s%d", names[idx], octave);
}

void setup() {
  // 1. Démarrage Série pour le debug
  Serial.begin(115200);
  delay(1000);
  Serial.println("--- TEST DUAL SCREEN ---");

  // 0. IO Matrice
  for (uint8_t c = 0; c < 14; ++c) {
    pinMode(COLS[c], INPUT_PULLUP);
  }
  for (uint8_t r = 0; r < 5; ++r) {
    pinMode(ROWS[r], OUTPUT);
    digitalWrite(ROWS[r], HIGH);
  }

  // 2. Initialisation Écran GAUCHE (0x3C)
  Serial.print("Init Ecran Gauche (0x3C)... ");
  leftEye.setI2CAddress(0x3C * 2); // U8g2 veut l'adresse * 2 (soit 0x78)
  leftEye.begin();
  leftEye.clearBuffer();
  leftEye.setFont(u8g2_font_ncenB14_tr);
  leftEye.drawStr(10, 40, "AZOTH");
  leftEye.sendBuffer();
  Serial.println("OK !");

  delay(500); // Petite pause de sécurité

  // 3. Initialisation Écran DROIT (0x3D)
  Serial.print("Init Ecran Droit (0x3D)... ");
  rightEye.setI2CAddress(0x3D * 2); // U8g2 veut l'adresse * 2 (soit 0x7A)
  rightEye.begin();
  rightEye.clearBuffer();
  rightEye.setFont(u8g2_font_ncenB14_tr);
  rightEye.drawStr(10, 40, "CREATION");
  rightEye.sendBuffer();
  Serial.println("OK !");

  // 4. Petit effet de démarrage : barre de progression synchronisée
  for (int w = 0; w <= 120; w += 6) {
    leftEye.clearBuffer();
    leftEye.setFont(u8g2_font_ncenB14_tr);
    leftEye.drawStr(10, 40, "AZOTH");
    leftEye.drawFrame(4, 52, 120, 8);
    leftEye.drawBox(4, 52, w, 8);
    leftEye.sendBuffer();

    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_ncenB14_tr);
    rightEye.drawStr(0, 40, "CREATION");
    rightEye.drawFrame(4, 52, 120, 8);
    rightEye.drawBox(4, 52, w, 8);
    rightEye.sendBuffer();

    delay(30);
  }

  // 5. Mode veille : yeux statiques centrés
  leftEye.clearBuffer();
  leftEye.drawCircle(64 - 3, 32, 30);
  leftEye.drawDisc(64 - 3, 32, 10);
  leftEye.sendBuffer();

  rightEye.clearBuffer();
  rightEye.drawCircle(64 + 3, 32, 30);
  rightEye.drawDisc(64 + 3, 32, 10);
  rightEye.sendBuffer();

  // Potentiomètres : initialisation des valeurs
  for (size_t i = 0; i < POT_COUNT; ++i) {
    potPrev[i] = analogRead(POT_PINS[i]);
  }
}

void loop() {
  // Scan matrice et log sur le terminal
  char noteName[8] = "";
  for (uint8_t r = 0; r < 5; ++r) {
    for (uint8_t rr = 0; rr < 5; ++rr) {
      digitalWrite(ROWS[rr], rr == r ? LOW : HIGH);
    }
    delayMicroseconds(40);

    for (uint8_t c = 0; c < 14; ++c) {
      const bool pressed = digitalRead(COLS[c]) == LOW;
      const uint8_t note = NOTE_MAP[r][c];
      if (pressed != prevState[r][c]) {
        noteNameFromNumber(note, noteName, sizeof(noteName));
        if (pressed) {
          Serial.print(F("KEY DOWN  row="));
        } else {
          Serial.print(F("KEY UP    row="));
        }
        Serial.print(r);
        Serial.print(F(" col="));
        Serial.print(c);
        Serial.print(F(" pin="));
        Serial.print(COLS[c]);
        Serial.print(F(" note="));
        Serial.println(noteName);
      }
      prevState[r][c] = pressed;
    }
  }
  for (uint8_t rr = 0; rr < 5; ++rr) {
    digitalWrite(ROWS[rr], HIGH);
  }

  // Lecture des potentiomètres (avec seuil pour éviter le bruit)
  for (size_t i = 0; i < POT_COUNT; ++i) {
    int val = analogRead(POT_PINS[i]);
    if (abs(val - potPrev[i]) > 8) { // seuil anti-jitter
      potPrev[i] = val;
      Serial.print(F("POT "));
      Serial.print(i);
      Serial.print(F(" pin="));
      Serial.print(POT_PINS[i]);
      Serial.print(F(" value="));
      Serial.println(val);
    }
  }

  delay(20);
}
