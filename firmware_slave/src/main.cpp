#include <Arduino.h>
#include <U8g2lib.h>
#include <Encoder.h>

#include "config.h"

// ---------------------------------------------------------------------------
// Configuration matrice (ordre exact demandé pour le diagnostic)
// Col 6 -> Pin 37, Col 7 -> Pin 33, Col 9 -> Pin 38 (fils de réparation)
// ---------------------------------------------------------------------------
constexpr uint8_t ROWS = 5;
constexpr uint8_t COLS = 14;
const uint8_t rowPins[ROWS] = {24, 23, 34, 35, 28};
const uint8_t colPins[COLS] = {9, 8, 7, 4, 3, 2, 37, 33, 25, 38, 13, 41, 40, 36};

// Joystick (brut) sur Pin 15 (A1/A15 suivant le repérage Teensy)
constexpr uint8_t JOY_PIN = 15;

// Encodeurs
Encoder enc1(5, 6);
Encoder enc2(26, 27);
Encoder enc3(29, 30);
Encoder enc4(31, 32);
long enc1Last = 0, enc2Last = 0, enc3Last = 0, enc4Last = 0;

// Ecran OLED (I2C 18/19)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE,
                                         /* clock=*/ OLED_SCL, /* data=*/ OLED_SDA);

// Etats précédents pour éviter le spam console
bool keyState[ROWS][COLS] = {};
int lastJoy = -1;
char lastKeyLabel[24] = "Rien";

void setupMatrix() {
    // Colonnes en INPUT_PULLUP, lignes en OUTPUT (repos HIGH)
    for (uint8_t c = 0; c < COLS; ++c) {
        pinMode(colPins[c], INPUT_PULLUP);
    }
    for (uint8_t r = 0; r < ROWS; ++r) {
        pinMode(rowPins[r], OUTPUT);
        digitalWrite(rowPins[r], HIGH);
    }
}

void logKey(uint8_t row, uint8_t col) {
    const uint8_t physPin = colPins[col];
    Serial.print(F("[KEY] Row: "));
    Serial.print(row);
    Serial.print(F(" | Col: "));
    Serial.print(col);
    Serial.print(F(" | Pin: "));
    Serial.println(physPin);

    if (physPin == 33 || physPin == 37 || physPin == 38) {
        Serial.println(F("[OK] SIGNAL RECU SUR PIN DE REPARATION"));
    }

    snprintf(lastKeyLabel, sizeof(lastKeyLabel), "R%u C%u P%u", row, col,
             physPin);
}

void scanMatrix() {
    for (uint8_t r = 0; r < ROWS; ++r) {
        // Active la ligne r
        for (uint8_t rr = 0; rr < ROWS; ++rr) {
            digitalWrite(rowPins[rr], rr == r ? LOW : HIGH);
        }

        delayMicroseconds(50); // temps de stabilisation

        for (uint8_t c = 0; c < COLS; ++c) {
            const bool pressed = digitalRead(colPins[c]) == LOW;
            if (pressed && !keyState[r][c]) {
                logKey(r, c);
            }
            keyState[r][c] = pressed;
        }
    }

    // Remet toutes les lignes au repos
    for (uint8_t rr = 0; rr < ROWS; ++rr) {
        digitalWrite(rowPins[rr], HIGH);
    }
}

void checkJoystick() {
    const int val = analogRead(JOY_PIN);
    if (lastJoy < 0 || abs(val - lastJoy) > 5) {
        Serial.print(F("[JOY] Value: "));
        Serial.println(val);
        lastJoy = val;
    }
}

void checkEncoders() {
    const long p1 = enc1.read();
    const long p2 = enc2.read();
    const long p3 = enc3.read();
    const long p4 = enc4.read();

    if (p1 != enc1Last) {
        Serial.print(F("[ENC] ID:1 | Pos: "));
        Serial.println(p1);
        enc1Last = p1;
    }
    if (p2 != enc2Last) {
        Serial.print(F("[ENC] ID:2 | Pos: "));
        Serial.println(p2);
        enc2Last = p2;
    }
    if (p3 != enc3Last) {
        Serial.print(F("[ENC] ID:3 | Pos: "));
        Serial.println(p3);
        enc3Last = p3;
    }
    if (p4 != enc4Last) {
        Serial.print(F("[ENC] ID:4 | Pos: "));
        Serial.println(p4);
        enc4Last = p4;
    }
}

void drawUI() {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tr);

    // Haut : dernière touche
    u8g2.drawStr(0, 12, "KEY:");
    u8g2.drawStr(32, 12, lastKeyLabel);

    // Milieu : joystick
    char joyText[20];
    snprintf(joyText, sizeof(joyText), "JOY:%4d", lastJoy >= 0 ? lastJoy : 0);
    u8g2.drawStr(0, 30, joyText);
    const uint8_t barWidth =
        static_cast<uint8_t>(map(lastJoy < 0 ? 0 : lastJoy, 0, 1023, 0, 120));
    u8g2.drawFrame(4, 34, 120, 10);
    u8g2.drawBox(4, 34, barWidth, 10);

    // Bas : positions des encodeurs
    char encText[48];
    snprintf(encText, sizeof(encText), "E1:%4ld E2:%4ld", enc1Last, enc2Last);
    u8g2.drawStr(0, 52, encText);
    snprintf(encText, sizeof(encText), "E3:%4ld E4:%4ld", enc3Last, enc4Last);
    u8g2.drawStr(0, 64, encText);

    u8g2.sendBuffer();
}

void setup() {
    Serial.begin(115200);
    analogReadResolution(10); // 0..1023

    setupMatrix();

    u8g2.begin();
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(0, 12, "CONSOLE DEBUG READY");
    u8g2.sendBuffer();
}

void loop() {
    scanMatrix();
    checkJoystick();
    checkEncoders();
    drawUI();
}
