#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Keypad.h>

#include "slave/config.h"

// ==========================================
// CONFIGURATION MATRICE (D'après config.h)
// ==========================================
const byte ROWS = 5;
const byte COLS = 14;

byte rowPins[ROWS] = {ROW_0, ROW_1, ROW_2, ROW_3, ROW_4};
byte colPins[COLS] = {COL_0,  COL_1,  COL_2,  COL_3,  COL_4,  COL_5, COL_6,
                      COL_7,  COL_8,  COL_9,  COL_10, COL_11, COL_12, COL_13};

// Mapping MIDI (exemple : complète selon ton besoin physique)
int midiMap[ROWS][COLS] = {
    {UNDO_BUTTON, TEMPO_BUTTON, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};

Keypad kpd = Keypad(makeKeymap(midiMap), rowPins, colPins, ROWS, COLS);

// ------------------- Écrans OLED -------------------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C leftEye(U8G2_R2, U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C rightEye(U8G2_R2, U8X8_PIN_NONE);

// ------------------- Helpers -------------------
void showKeyEvent(bool pressed, int code) {
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_6x12_tr);
    rightEye.drawStr(0, 14, pressed ? "KEY DOWN" : "KEY UP");
    char buf[24];
    snprintf(buf, sizeof(buf), "CODE: %d", code);
    rightEye.drawStr(0, 30, buf);
    rightEye.sendBuffer();
}

void drawBoot() {
    Serial.println("--- TEST DUAL SCREEN ---");

    Serial.print("Init Ecran Gauche (0x3C)... ");
    leftEye.setI2CAddress(0x3C * 2);
    leftEye.begin();
    leftEye.clearBuffer();
    leftEye.setFont(u8g2_font_ncenB14_tr);
    leftEye.drawStr(10, 40, "AZOTH");
    leftEye.sendBuffer();
    Serial.println("OK !");

    delay(500);

    Serial.print("Init Ecran Droit (0x3D)... ");
    rightEye.setI2CAddress(0x3D * 2);
    rightEye.begin();
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_ncenB14_tr);
    rightEye.drawStr(10, 40, "CREATION");
    rightEye.sendBuffer();
    Serial.println("OK !");

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

    leftEye.clearBuffer();
    leftEye.drawCircle(64 - 3, 32, 30);
    leftEye.drawDisc(64 - 3, 32, 10);
    leftEye.sendBuffer();

    rightEye.clearBuffer();
    rightEye.drawCircle(64 + 3, 32, 30);
    rightEye.drawDisc(64 + 3, 32, 10);
    rightEye.sendBuffer();
}

void setupControls() {
    pinMode(JOYSTICK_X_PIN, INPUT);
    pinMode(VOLUME_POT_PIN, INPUT);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    setupControls();
    drawBoot();
}

void loop() {
    if (kpd.getKeys()) {
        for (int i = 0; i < LIST_MAX; i++) {
            if (kpd.key[i].kstate == PRESSED || kpd.key[i].kstate == RELEASED) {
                Serial.print(kpd.key[i].kstate == PRESSED ? F("KEY DOWN ") : F("KEY UP   "));
                Serial.print(F("code="));
                Serial.println(kpd.key[i].kchar);
                showKeyEvent(kpd.key[i].kstate == PRESSED, kpd.key[i].kchar);
            }
        }
    }

    // Lecture analogiques (sécurisées)
    int joy = analogRead(JOYSTICK_X_PIN);
    int vol = analogRead(VOLUME_POT_PIN);
    static int joyPrev = -1;
    static int volPrev = -1;
    if (joyPrev < 0 || abs(joy - joyPrev) > 8) {
        joyPrev = joy;
        Serial.print(F("JOY X="));
        Serial.println(joy);
    }
    if (volPrev < 0 || abs(vol - volPrev) > 8) {
        volPrev = vol;
        Serial.print(F("VOL =" ));
        Serial.println(vol);
    }

    delay(20);
}
