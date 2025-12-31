#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>

#include "slave/config.h"

// ------------------- Matrice Easy-Wire -------------------
constexpr uint8_t ROW_COUNT = 5;
constexpr uint8_t COL_COUNT = 14;

const int rowPins[ROW_COUNT] = {ROW_0, ROW_1, ROW_2, ROW_3, ROW_4};
const int colPins[COL_COUNT] = {COL_0,  COL_1,  COL_2,  COL_3,  COL_4,  COL_5,  COL_6,
                                COL_7,  COL_8,  COL_9,  COL_10, COL_11, COL_12, COL_13};

// Mapping MIDI/CC par touche (à compléter selon besoin)
const int midiMap[ROW_COUNT][COL_COUNT] = {
    {UNDO_BUTTON, TEMPO_BUTTON, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

bool prevState[ROW_COUNT][COL_COUNT] = {};

// ------------------- Écrans OLED -------------------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C leftEye(U8G2_R2, U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C rightEye(U8G2_R2, U8X8_PIN_NONE);

// ------------------- Helpers -------------------
void drawLeftEye(int joyVal, int volVal) {
    leftEye.clearBuffer();
    leftEye.drawCircle(64, 32, 30);
    const int pupilX = map(joyVal, 0, 1023, 64 - 15, 64 + 15);
    leftEye.drawDisc(pupilX, 32, 10);
    leftEye.drawFrame(4, 54, 120, 8);
    leftEye.drawBox(4, 54, map(volVal, 0, 1023, 0, 120), 8);
    leftEye.sendBuffer();
}

void showKeyEvent(bool pressed, int code, int row, int col, int pin) {
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_6x12_tr);
    rightEye.drawStr(0, 12, pressed ? "KEY DOWN" : "KEY UP");
    char buf[24];
    snprintf(buf, sizeof(buf), "CODE: %d", code);
    rightEye.drawStr(0, 24, buf);
    snprintf(buf, sizeof(buf), "R%u C%u", row, col);
    rightEye.drawStr(0, 36, buf);
    snprintf(buf, sizeof(buf), "PIN:%d", pin);
    rightEye.drawStr(0, 48, buf);
    rightEye.sendBuffer();
}

void drawBoot() {
    Serial.println("--- TEST DUAL SCREEN ---");

    Serial.print("Init Ecran Gauche (0x3C)... ");
    leftEye.setI2CAddress(0x3C * 2);
    leftEye.begin();
    drawLeftEye(512, 0); // centre par défaut
    Serial.println("OK !");

    delay(300);

    Serial.print("Init Ecran Droit (0x3D)... ");
    rightEye.setI2CAddress(0x3D * 2);
    rightEye.begin();
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_ncenB14_tr);
    rightEye.drawStr(0, 40, "READY");
    rightEye.sendBuffer();
    Serial.println("OK !");
}

void setupControls() {
    pinMode(JOYSTICK_X_PIN, INPUT);
    pinMode(VOLUME_POT_PIN, INPUT);
}

void setupMatrixIO() {
    for (uint8_t c = 0; c < COL_COUNT; ++c) {
        pinMode(colPins[c], INPUT_PULLUP);
    }
    for (uint8_t r = 0; r < ROW_COUNT; ++r) {
        pinMode(rowPins[r], OUTPUT);
        digitalWrite(rowPins[r], HIGH);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    setupControls();
    setupMatrixIO();
    drawBoot();
}

void loop() {
    // Lecture analogiques
    static int joyPrev = -1;
    static int volPrev = -1;
    int joy = analogRead(JOYSTICK_X_PIN);
    int vol = analogRead(VOLUME_POT_PIN);

    if (joyPrev < 0 || abs(joy - joyPrev) > 8) {
        joyPrev = joy;
    }
    if (volPrev < 0 || abs(vol - volPrev) > 8) {
        volPrev = vol;
    }
    drawLeftEye(joyPrev, volPrev);

    // Scan matrice manuel
    for (uint8_t r = 0; r < ROW_COUNT; ++r) {
        for (uint8_t rr = 0; rr < ROW_COUNT; ++rr) {
            digitalWrite(rowPins[rr], rr == r ? LOW : HIGH);
        }
        delayMicroseconds(40);

        for (uint8_t c = 0; c < COL_COUNT; ++c) {
            const bool pressed = (digitalRead(colPins[c]) == LOW);
            if (pressed != prevState[r][c]) {
                const int code = midiMap[r][c];
                showKeyEvent(pressed, code, r, c, colPins[c]);
            }
            prevState[r][c] = pressed;
        }
    }
    for (uint8_t rr = 0; rr < ROW_COUNT; ++rr) {
        digitalWrite(rowPins[rr], HIGH);
    }

    delay(20);
}
