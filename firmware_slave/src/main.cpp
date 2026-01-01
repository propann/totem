#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Encoder.h>

#include "slave/config.h"

// ============================================================
// 1. ENCODEURS (configuration finale)
// ============================================================
Encoder enc1(5, 6);      // Encodeur 1 validé
Encoder enc2(29, 30);    // Encodeur 2 (supposition, proche de 31/32)
Encoder enc3(31, 32);    // Encodeur 3 (menu)
Encoder enc4(26, 27);    // Encodeur 4 validé

long pos1 = -999, pos2 = -999, pos3 = -999, pos4 = -999;

// ============================================================
// 2. MATRICE EASY-WIRE
// ============================================================
constexpr uint8_t ROW_COUNT = 5;
constexpr uint8_t COL_COUNT = 14;

const int rowPins[ROW_COUNT] = {ROW_0, ROW_1, ROW_2, ROW_3, ROW_4};
const int colPins[COL_COUNT] = {COL_0,  COL_1,  COL_2,  COL_3,  COL_4,  COL_5,  COL_6,
                                COL_7,  COL_8,  COL_9,  COL_10, COL_11, COL_12, COL_13};

// Mapping CC (à compléter si besoin)
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
// 4. ÉTATS DU SYSTÈME
// ============================================================
enum SystemState { STATE_MENU, STATE_TEST_HARDWARE, STATE_INFO };
SystemState currentState = STATE_MENU;
int menuSelection = 0;              // 0 = Start, 1 = Hardware Test, 2 = Info
const int MENU_ITEMS = 3;
long oldMenuEncPos = 0;             // position précédente de l'encodeur de menu (enc3)

// ============================================================
// 5. AFFICHAGE MENU / INFO
// ============================================================
void drawMenu() {
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_ncenB10_tr);
    rightEye.drawStr(20, 15, "LMN-3 SYSTEM");
    rightEye.drawHLine(0, 18, 128);
    rightEye.setFont(u8g2_font_6x12_tr);
    int yStart = 35, h = 12;
    rightEye.drawStr(5, yStart,        menuSelection == 0 ? "> START DAW" : "  START DAW");
    rightEye.drawStr(5, yStart + h,    menuSelection == 1 ? "> HARDWARE TEST" : "  HARDWARE TEST");
    rightEye.drawStr(5, yStart + h*2,  menuSelection == 2 ? "> INFO / CREDITS" : "  INFO / CREDITS");
    rightEye.sendBuffer();

    leftEye.clearBuffer();
    leftEye.setFont(u8g2_font_ncenB14_tr);
    leftEye.drawStr(20, 40, "MENU");
    leftEye.sendBuffer();
}

void drawInfo() {
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_6x12_tr);
    rightEye.drawStr(0, 10, "LMN-3 CUSTOM");
    rightEye.drawStr(0, 25, "FW: v2 Menu");
    rightEye.drawStr(0, 40, "Dev: Codex & You");
    rightEye.drawStr(0, 60, "[TOUCHE POUR EXIT]");
    rightEye.sendBuffer();
}

// ============================================================
// 6. TEST HARDWARE
// ============================================================
void runHardwareTest() {
    static int joyPrev = -1;
    int joy = analogRead(JOYSTICK_X_PIN);
    if (abs(joy - joyPrev) > 8) {
        joyPrev = joy;
        leftEye.clearBuffer();
        leftEye.setFont(u8g2_font_9x15B_tr);
        leftEye.drawStr(0, 15, "TEST MODE");
        leftEye.drawFrame(0, 30, 128, 20);
        leftEye.drawBox(0, 30, map(joy, 0, 1023, 0, 128), 20);
        leftEye.sendBuffer();
    }

    long n1 = enc1.read();
    long n2 = enc2.read();
    long n3 = enc3.read();
    long n4 = enc4.read();

    if (n1 != pos1) { pos1 = n1; rightEye.clearBuffer(); rightEye.setFont(u8g2_font_ncenB14_tr); rightEye.setCursor(0,40); rightEye.print("ENC 1: "); rightEye.print(n1); rightEye.sendBuffer(); }
    if (n2 != pos2) { pos2 = n2; rightEye.clearBuffer(); rightEye.setFont(u8g2_font_ncenB14_tr); rightEye.setCursor(0,40); rightEye.print("ENC 2: "); rightEye.print(n2); rightEye.sendBuffer(); }
    if (n3 != pos3) { pos3 = n3; rightEye.clearBuffer(); rightEye.setFont(u8g2_font_ncenB14_tr); rightEye.setCursor(0,40); rightEye.print("ENC 3: "); rightEye.print(n3); rightEye.sendBuffer(); }
    if (n4 != pos4) { pos4 = n4; rightEye.clearBuffer(); rightEye.setFont(u8g2_font_ncenB14_tr); rightEye.setCursor(0,40); rightEye.print("ENC 4: "); rightEye.print(n4); rightEye.sendBuffer(); }

    bool exitToMenu = false;
    for (uint8_t r = 0; r < ROW_COUNT; ++r) {
        for (uint8_t rr = 0; rr < ROW_COUNT; ++rr) digitalWrite(rowPins[rr], rr == r ? LOW : HIGH);
        delayMicroseconds(40);
        for (uint8_t c = 0; c < COL_COUNT; ++c) {
            const bool pressed = (digitalRead(colPins[c]) == LOW);
            if (pressed != prevState[r][c]) {
                rightEye.clearBuffer();
                rightEye.setFont(u8g2_font_6x12_tr);
                rightEye.drawStr(0, 15, pressed ? "KEY DOWN" : "KEY UP");
                rightEye.setCursor(0, 30); rightEye.print("PIN: "); rightEye.print(colPins[c]);
                rightEye.setCursor(0, 45); rightEye.print("CC: "); rightEye.print(midiMap[r][c]);
                rightEye.sendBuffer();
                if (colPins[c] == 40 && pressed) {
                    exitToMenu = true; // Bouton enc2 pressé -> retour menu
                }
            }
            prevState[r][c] = pressed;
        }
    }
    for (uint8_t rr = 0; rr < ROW_COUNT; ++rr) digitalWrite(rowPins[rr], HIGH);
    if (exitToMenu) {
        currentState = STATE_MENU;
        drawMenu();
    }
}

// ============================================================
// 7. SETUP
// ============================================================
void setup() {
    Serial.begin(115200);
    pinMode(JOYSTICK_X_PIN, INPUT);
    for (uint8_t c = 0; c < COL_COUNT; ++c) pinMode(colPins[c], INPUT_PULLUP);
    for (uint8_t r = 0; r < ROW_COUNT; ++r) { pinMode(rowPins[r], OUTPUT); digitalWrite(rowPins[r], HIGH); }

    leftEye.setI2CAddress(0x3C * 2); leftEye.begin();
    rightEye.setI2CAddress(0x3D * 2); rightEye.begin();

    drawMenu();
}

// ============================================================
// 8. LOOP PRINCIPAL
// ============================================================
void loop() {
    if (currentState == STATE_MENU) {
        // Navigation avec enc3 (31/32)
        long newPos = enc3.read() / 4;
        if (newPos != oldMenuEncPos) {
            if (newPos > oldMenuEncPos) menuSelection++;
            else menuSelection--;
            if (menuSelection >= MENU_ITEMS) menuSelection = 0;
            if (menuSelection < 0) menuSelection = MENU_ITEMS - 1;
            oldMenuEncPos = newPos;
            drawMenu();
        }

        // Validation : toute touche de la matrice
        bool anyKeyPressed = false;
        for (uint8_t r = 0; r < ROW_COUNT; ++r) {
            digitalWrite(rowPins[r], LOW);
            delayMicroseconds(20);
            for (uint8_t c = 0; c < COL_COUNT; ++c) {
                if (digitalRead(colPins[c]) == LOW) {
                    anyKeyPressed = true;
                    while (digitalRead(colPins[c]) == LOW) { /* attendre relâche */ }
                }
            }
            digitalWrite(rowPins[r], HIGH);
        }

        if (anyKeyPressed) {
            if (menuSelection == 0) {
                rightEye.clearBuffer(); rightEye.drawStr(10,30,"DAW LOADING..."); rightEye.sendBuffer();
                delay(1000);
                drawMenu();
            } else if (menuSelection == 1) {
                currentState = STATE_TEST_HARDWARE;
                leftEye.clearBuffer(); leftEye.drawStr(0, 30, "TEST STARTED"); leftEye.sendBuffer();
                rightEye.clearBuffer(); rightEye.drawStr(0, 30, "RESET TO EXIT"); rightEye.sendBuffer();
                delay(1000);
            } else if (menuSelection == 2) {
                currentState = STATE_INFO;
                drawInfo();
            }
        }
    }
    else if (currentState == STATE_INFO) {
        bool click = false;
        for (uint8_t r = 0; r < ROW_COUNT; ++r) {
            digitalWrite(rowPins[r], LOW); delayMicroseconds(20);
            for (uint8_t c = 0; c < COL_COUNT; ++c) {
                if (digitalRead(colPins[c]) == LOW) { click = true; while (digitalRead(colPins[c]) == LOW) {} }
            }
            digitalWrite(rowPins[r], HIGH);
        }
        if (click) { currentState = STATE_MENU; drawMenu(); }
    }
    else if (currentState == STATE_TEST_HARDWARE) {
        runHardwareTest();
        // Sortie : reset physique
    }
}
