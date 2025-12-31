#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Encoder.h>

#include "slave/config.h"

// ============================================================
// 1. CONFIGURATION DES ENCODEURS (Chasse finale)
// ============================================================
// Confirmés
Encoder enc1(5, 6);     // Encodeur 1 OK
Encoder enc3(31, 32);   // Encodeur 3 OK
Encoder enc4(26, 27);   // Encodeur 4 OK

// Pistes pour l’encodeur manquant
Encoder enc2(29, 30);   // Piste A : juste à côté de 31/32
Encoder enc5(10, 11);   // Piste B : SPI
Encoder enc6(20, 21);   // Piste C : Audio

long pos1 = -999, pos2 = -999, pos3 = -999, pos4 = -999;
long pos5 = -999, pos6 = -999;

// ============================================================
// 2. CONFIGURATION MATRICE
// ============================================================
constexpr uint8_t ROW_COUNT = 5;
constexpr uint8_t COL_COUNT = 14;

const int rowPins[ROW_COUNT] = {ROW_0, ROW_1, ROW_2, ROW_3, ROW_4};
const int colPins[COL_COUNT] = {COL_0,  COL_1,  COL_2,  COL_3,  COL_4,  COL_5,  COL_6,
                                COL_7,  COL_8,  COL_9,  COL_10, COL_11, COL_12, COL_13};

// Mapping MIDI (pour affichage)
const int midiMap[ROW_COUNT][COL_COUNT] = {
    {UNDO_BUTTON, TEMPO_BUTTON, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

bool prevState[ROW_COUNT][COL_COUNT] = {};

// ============================================================
// 3. ÉCRANS OLED
// ============================================================
U8G2_SSD1306_128X64_NONAME_F_HW_I2C leftEye(U8G2_R2, U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C rightEye(U8G2_R2, U8X8_PIN_NONE);

// ============================================================
// 4. FONCTIONS D'AFFICHAGE
// ============================================================
void drawLeftBars(int joyVal) {
    leftEye.clearBuffer();
    leftEye.setFont(u8g2_font_9x15B_tr);
    leftEye.drawStr(0, 15, "JOYSTICK X");
    leftEye.drawFrame(0, 30, 128, 20);
    int width = map(joyVal, 0, 1023, 0, 128);
    leftEye.drawBox(0, 30, width, 20);
    leftEye.sendBuffer();
}

void showKeyEvent(bool pressed, int code, int pin) {
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_6x12_tr);
    rightEye.drawStr(0, 15, pressed ? "KEY DOWN" : "KEY UP");
    char buf[30];
    snprintf(buf, sizeof(buf), "PIN PHY: %d", pin);
    rightEye.drawStr(0, 35, buf);
    snprintf(buf, sizeof(buf), "CC: %d", code);
    rightEye.drawStr(0, 55, buf);
    rightEye.sendBuffer();
}

void showEncoderEvent(const char *label, long value, const char *pinInfo) {
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_9x15B_tr);
    rightEye.drawStr(0, 20, label);
    rightEye.setFont(u8g2_font_6x12_tr);
    rightEye.drawStr(0, 40, pinInfo);
    rightEye.setFont(u8g2_font_ncenB18_tr);
    char bufVal[20];
    snprintf(bufVal, sizeof(bufVal), "%ld", value);
    rightEye.drawStr(40, 64, bufVal);
    rightEye.sendBuffer();
}

void setup() {
    Serial.begin(115200);
    pinMode(JOYSTICK_X_PIN, INPUT);

    // Matrice
    for (uint8_t c = 0; c < COL_COUNT; ++c) pinMode(colPins[c], INPUT_PULLUP);
    for (uint8_t r = 0; r < ROW_COUNT; ++r) {
        pinMode(rowPins[r], OUTPUT);
        digitalWrite(rowPins[r], HIGH);
    }

    // Écrans
    leftEye.setI2CAddress(0x3C * 2); leftEye.begin(); drawLeftBars(512);
    rightEye.setI2CAddress(0x3D * 2); rightEye.begin();
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_ncenB14_tr);
    rightEye.drawStr(10, 40, "CHASSE V4");
    rightEye.sendBuffer();
}

void loop() {
    // 1. JOYSTICK
    static int joyPrev = -1;
    int joy = analogRead(JOYSTICK_X_PIN);
    if (abs(joy - joyPrev) > 8) { joyPrev = joy; drawLeftBars(joy); }

    // 2. ENCODEURS (détection en parallèle)
    long n1 = enc1.read();
    long n2 = enc2.read();
    long n3 = enc3.read();
    long n4 = enc4.read();
    long n5 = enc5.read();
    long n6 = enc6.read();

    if (n1 != pos1) { pos1 = n1; showEncoderEvent("ENC 1 (OK)", n1, "Pins 5-6"); }
    if (n2 != pos2) { pos2 = n2; showEncoderEvent("BINGO! (E2)", n2, "Pins 29-30"); }
    if (n3 != pos3) { pos3 = n3; showEncoderEvent("ENC 3 (OK)", n3, "Pins 31-32"); }
    if (n4 != pos4) { pos4 = n4; showEncoderEvent("ENC 4 (OK)", n4, "Pins 26-27"); }
    if (n5 != pos5) { pos5 = n5; showEncoderEvent("BINGO! (E5)", n5, "Pins 10-11"); }
    if (n6 != pos6) { pos6 = n6; showEncoderEvent("BINGO! (E6)", n6, "Pins 20-21"); }

    // 3. MATRICE
    for (uint8_t r = 0; r < ROW_COUNT; ++r) {
        for (uint8_t rr = 0; rr < ROW_COUNT; ++rr) digitalWrite(rowPins[rr], rr == r ? LOW : HIGH);
        delayMicroseconds(40);
        for (uint8_t c = 0; c < COL_COUNT; ++c) {
            const bool pressed = (digitalRead(colPins[c]) == LOW);
            if (pressed != prevState[r][c]) {
                showKeyEvent(pressed, midiMap[r][c], colPins[c]);
            }
            prevState[r][c] = pressed;
        }
    }
    for (uint8_t rr = 0; rr < ROW_COUNT; ++rr) digitalWrite(rowPins[rr], HIGH);

    delay(10);
}
