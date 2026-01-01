#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Encoder.h>

#include "slave/config.h"

// ============================================================
// 1. CONFIGURATION MATÉRIELLE (CORRIGÉE)
// ============================================================

// --- ENCODEURS ---

// Encodeur 1 (Gauche) - Validé
Encoder enc1(5, 6);   

// Encodeur 2 (L'avant-dernier, celui qui manque)
// On laisse 29, 30 en "supposition". Si ça ne marche pas, ce n'est pas grave pour le menu.
Encoder enc2(29, 30); 

// Encodeur 3 (Tout à droite - LE CHEF DU MENU) - CORRIGÉ
Encoder enc3(31, 32); 

// Encodeur 4 (Le 2ème en partant de la gauche ?) - Validé
Encoder enc4(26, 27); 


// Variables de position pour le Test
long pos1 = -999, pos2 = -999, pos3 = -999, pos4 = -999;

// --- MATRICE ---
constexpr uint8_t ROW_COUNT = 5;
constexpr uint8_t COL_COUNT = 14;
const int rowPins[ROW_COUNT] = {ROW_0, ROW_1, ROW_2, ROW_3, ROW_4};
const int colPins[COL_COUNT] = {COL_0,  COL_1,  COL_2,  COL_3,  COL_4,  COL_5,  COL_6,
                                COL_7,  COL_8,  COL_9,  COL_10, COL_11, COL_12, COL_13};

// Mapping MIDI (Visuel)
const int midiMap[ROW_COUNT][COL_COUNT] = {
    {UNDO_BUTTON, TEMPO_BUTTON, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
    {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
};
bool prevState[ROW_COUNT][COL_COUNT] = {};

// --- ECRANS ---
U8G2_SSD1306_128X64_NONAME_F_HW_I2C leftEye(U8G2_R2, U8X8_PIN_NONE);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C rightEye(U8G2_R2, U8X8_PIN_NONE);

// ============================================================
// 2. SYSTÈME DE MENU (LOGIQUE)
// ============================================================
enum SystemState {
    STATE_MENU,
    STATE_TEST_HARDWARE,
    STATE_INFO
};

SystemState currentState = STATE_MENU;
int menuSelection = 0; // 0 = Start, 1 = Test, 2 = Info
const int MENU_ITEMS = 3;

// Variables pour le menu
long oldMenuEncPos = 0;

// ============================================================
// 3. FONCTIONS GRAPHIQUES
// ============================================================

void drawMenu() {
    rightEye.clearBuffer();
    
    // Titre
    rightEye.setFont(u8g2_font_ncenB10_tr);
    rightEye.drawStr(20, 15, "LMN-3 SYSTEM");
    rightEye.drawHLine(0, 18, 128);

    // Options
    rightEye.setFont(u8g2_font_6x12_tr);
    
    // Curseur
    int yStart = 35;
    int h = 12;
    rightEye.drawStr(5, yStart,     menuSelection == 0 ? "> START DAW" : "  START DAW");
    rightEye.drawStr(5, yStart + h, menuSelection == 1 ? "> HARDWARE TEST" : "  HARDWARE TEST");
    rightEye.drawStr(5, yStart + h*2, menuSelection == 2 ? "> INFO / CREDITS" : "  INFO / CREDITS");

    rightEye.sendBuffer();

    // Ecran gauche Logo
    leftEye.clearBuffer();
    leftEye.setFont(u8g2_font_ncenB14_tr);
    leftEye.drawStr(20, 40, "MENU");
    leftEye.sendBuffer();
}

void drawInfo() {
    rightEye.clearBuffer();
    rightEye.setFont(u8g2_font_6x12_tr);
    rightEye.drawStr(0, 10, "LMN-3 CUSTOM");
    rightEye.drawStr(0, 25, "FW: v1.0 Menu");
    rightEye.drawStr(0, 40, "Dev: Codex & You");
    rightEye.drawStr(0, 60, "[CLICK TO EXIT]");
    rightEye.sendBuffer();
}

// ============================================================
// 4. LE TEST HARDWARE (SÉCURISÉ)
// ============================================================
void runHardwareTest() {
    // A. JOYSTICK & BARRES (Ecran Gauche)
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

    // B. ENCODEURS
    long n1 = enc1.read();
    long n2 = enc2.read();
    long n3 = enc3.read();
    long n4 = enc4.read();

    if (n1 != pos1) { 
        pos1 = n1; 
        rightEye.clearBuffer(); rightEye.setFont(u8g2_font_ncenB14_tr);
        rightEye.setCursor(0,40); rightEye.print("ENC 1: "); rightEye.print(n1); rightEye.sendBuffer();
    }
    if (n2 != pos2) { 
        pos2 = n2; 
        rightEye.clearBuffer(); rightEye.setFont(u8g2_font_ncenB14_tr);
        rightEye.setCursor(0,40); rightEye.print("ENC 2: "); rightEye.print(n2); rightEye.sendBuffer();
    }
    if (n3 != pos3) { 
        pos3 = n3; 
        rightEye.clearBuffer(); rightEye.setFont(u8g2_font_ncenB14_tr);
        rightEye.setCursor(0,40); rightEye.print("ENC 3: "); rightEye.print(n3); rightEye.sendBuffer();
    }
    if (n4 != pos4) { 
        pos4 = n4; 
        rightEye.clearBuffer(); rightEye.setFont(u8g2_font_ncenB14_tr);
        rightEye.setCursor(0,40); rightEye.print("ENC 4: "); rightEye.print(n4); rightEye.sendBuffer();
    }

    // C. MATRICE (Scan)
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
            }
            prevState[r][c] = pressed;
        }
    }
    for (uint8_t rr = 0; rr < ROW_COUNT; ++rr) digitalWrite(rowPins[rr], HIGH);
}


// ============================================================
// 5. SETUP & LOOP PRINCIPAL
// ============================================================

void setup() {
    Serial.begin(115200);
    pinMode(JOYSTICK_X_PIN, INPUT);

    // Matrice Init
    for (uint8_t c = 0; c < COL_COUNT; ++c) pinMode(colPins[c], INPUT_PULLUP);
    for (uint8_t r = 0; r < ROW_COUNT; ++r) {
        pinMode(rowPins[r], OUTPUT);
        digitalWrite(rowPins[r], HIGH);
    }
    
    // Ecrans Init
    leftEye.setI2CAddress(0x3C * 2); leftEye.begin();
    rightEye.setI2CAddress(0x3D * 2); rightEye.begin();
    
    drawMenu();
}

void loop() {
    
    // --- MODE MENU ---
    if (currentState == STATE_MENU) {
        // 1. Navigation avec ENCODEUR 3 (31, 32)
        long newPos = enc3.read() / 4; // Divisé par 4 pour la sensibilité
        if (newPos != oldMenuEncPos) {
            if (newPos > oldMenuEncPos) menuSelection++;
            else menuSelection--;
            
            // Boucle du menu
            if (menuSelection >= MENU_ITEMS) menuSelection = 0;
            if (menuSelection < 0) menuSelection = MENU_ITEMS - 1;
            
            oldMenuEncPos = newPos;
            drawMenu();
        }

        // 2. Detection Validation (N'importe quelle touche Matrice)
        bool anyKeyPressed = false;
        for (uint8_t r = 0; r < ROW_COUNT; ++r) {
            digitalWrite(rowPins[r], LOW);
            delayMicroseconds(20);
            for (uint8_t c = 0; c < COL_COUNT; ++c) {
                if (digitalRead(colPins[c]) == LOW) {
                    anyKeyPressed = true;
                    while(digitalRead(colPins[c]) == LOW); // Attente relâchement
                }
            }
            digitalWrite(rowPins[r], HIGH);
        }

        if (anyKeyPressed) {
            if (menuSelection == 0) {
                // START DAW
                rightEye.clearBuffer(); rightEye.drawStr(10,30,"DAW LOADING..."); rightEye.sendBuffer();
                delay(1000);
                drawMenu();
            }
            else if (menuSelection == 1) {
                // START TEST
                currentState = STATE_TEST_HARDWARE;
                leftEye.clearBuffer(); leftEye.drawStr(0, 30, "TEST STARTED"); leftEye.sendBuffer();
                rightEye.clearBuffer(); rightEye.drawStr(0, 30, "RESET TO EXIT"); rightEye.sendBuffer();
                delay(1000);
            }
            else if (menuSelection == 2) {
                // INFO
                currentState = STATE_INFO;
                drawInfo();
            }
        }
    }
    
    // --- MODE INFO ---
    else if (currentState == STATE_INFO) {
         // Clic pour sortir
         bool click = false;
         for (uint8_t r = 0; r < ROW_COUNT; ++r) {
            digitalWrite(rowPins[r], LOW); delayMicroseconds(20);
            for (uint8_t c = 0; c < COL_COUNT; ++c) {
                if (digitalRead(colPins[c]) == LOW) { click = true; while(digitalRead(colPins[c]) == LOW); }
            }
            digitalWrite(rowPins[r], HIGH);
        }
        if (click) {
            currentState = STATE_MENU;
            drawMenu();
        }
    }

    // --- MODE TEST ---
    else if (currentState == STATE_TEST_HARDWARE) {
        runHardwareTest();
        // Appuie sur RESET physique pour quitter le test
    }
}